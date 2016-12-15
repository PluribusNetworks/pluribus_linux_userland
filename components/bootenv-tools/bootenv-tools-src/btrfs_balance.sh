#!/bin/bash
#
# COPYRIGHT 2016 Pluribus Networks Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#
# Background
# ----------
#
# Btrfs is an extent based filesystem and grabs contiguous disk space
# in chunks or block groups.
#
# There are several different kinds of data that Btrfs stores on disk:
# 1) Metadata
# 2) File data
# 3) Internal System data
#
# For each of these categories separate chunks are allocated. The sizes
# of chunks for each category are different: Metadata - 256MB, File
# Data - 1GB, System - 32MB. These sizes are scaled down based on remaining
# free space on disk.
# Each chunk is marked with the category and holds only the same kind of
# data. For example File Data and Metadata are not mixed into the same
# chunk unless explicitly requested or disk size is 1GB or less.
#
# After chunks are allocated, actual blocks are allocated within the free
# space available inside the chunk. Typically, free space created after
# deletion are re-used for future block allocation for the same type of
# blocks. That is data blocks will never be allocated from Metadata chunks
# even if the latter has space available.
#
# Generally this allocation approach works fine, but In some cases it is
# possible for one chunk type to have eaten up all the free space  on disk.
#
# Consider the following scenario:
# Lets assume that a large bunch of huge files are copied and the Data chunks
# filled up all the physical free space on the disk.
# Now lots of files are deleted, so the chunks got freed up significantly.
# At this point we have lots of partially filled chunks. Now the user goes
# and copies a huge number of small files and creates a bunch of snapshots.
# So there is more metadata and Btrfs needs to allocate new Metadata chunks
# but it can't because all the free space is blocked into data chunks.
# At this point it starts giving ENOSPC errors even though free space is
# available inside data chunks. They are not available for Metadata.
#
# The typical symptom of this issue appears in the form of 'df' showing
# free space available and the user still getting ENOSPC. The output from
# df includes allocated but unused space and uallocated space. To get the
# real picture one can run "btrfs fi df /" for details and "btrfs fi show /"
# for an overview.
#
# This situation can be avoided by some kind of block reclaim algorithm which
# is still under development for Btrfs. So the alternative is to run a usage
# based balance operation which can re-distribute blocks from partially
# filled chunks into other partially filled chunks thereby freeing up some
# chunks to zero occupancy so that they can be removed. This creates free
# space on disk available for other chunk type allocations.
#
# The balance operation has to be done regularly. If the filesystem ever
# gets wedged into an ENOSPC situation with 100% allocation it can be
# impossible to recover from that because this balance operation on a
# copy-on-write filesystem needs to allocate new block group space which
# it will not find because everything is already grabbed. The only option
# is to re-format.
#
# -------------
#
# This script attempts to avoid this scenario and do a partial usage based
# balance operation with as low impact to running processes as possible.
# This script is run frequently to keep doing small rebalances and have
# some physical free space available unless the disk is really utilized
# heavily.
#


ROOTFS=/.rootbe

USED=0
MAXUSE=0
ALLOC=0

# Maximum free space wasted as percentage
MAX_WASTED_RATIO=0.4
# Target max waste when rebalancing (not much less than max_wasted)
TARGET_WASTED_RATIO=0.36

META_TARGET_WASTED_RATIO=0.50
MAX_FAILURES=20
MAX_REBALANCES=40
MAX_TARGET_THRESH1=60
MAX_TARGET_THRESH2=70
MAX_META_TARGET=50

#
# Sometimes we get stuck in a situation where the balance operation
# does some work but the target cutoff is not enough to give a net
# gain.
# This causes it to do wasted work at every run till the disk usages
# happen to change somewhat. So there is code to detect this situation
# and avoid wasted work:
#
# 1) Cache the disk utilization values (size, allocated, allocated but
#    unused, last work threshold)
# 2) Next time check what work threshold was cutoff and if any change
#    is seen in the disk usages.
# 3) If there is no change repeat balance with a higher work cutoff.
# 4) Next time repeat check from step #2 again.
# 5) If there is still no gain, stop doing anything.
# 6) This situation holds as long as the disk usages do not change. If
#    they do change, start again at step #1.
#
# These will get cached values from last run.
#
CACHED_TOT=0
CACHED_USED=0
CACHED_ALLOC=0
CACHED_TARGET=0

#
# Default Btrfs commit interval is 30 seconds
#
FS_COMMIT_INTERVAL=30

#
# Now query if we are changing it for '/'
#
intv=`cat /proc/mounts | awk '/\/ btrfs/ { if (match($4, /commit=([0-9]+)/, a) > 0) { print a[1]; }}'`
if [ "x$intv" != "x" ]
then
	FS_COMMIT_INTERVAL=$intv
fi

NVOS_LOG=/var/nvOS/log
LOGF=${NVOS_LOG}/btrfs_balance.log
ABORT_FILE=/run/btrfs_balance_abort
CACHE_FILE=/var/nvOS/log/btrfs_balance.cache
ROTATE_THRESH=$((10 * 1024 * 1024))
DID_RUN_BALANCE=0
exit_code=0

TYPE_META="Meta"
TYPE_DATA="Data"

#
# Globals used for running computations
#
meta_free_wasted=0
data_free_wasted=0

#
# Catch all script errors to log file.
#
exec 2>>$LOGF

#
# If abort has been signaled within the last 24 hrs don't do anything.
# This can happen during software upgrade.
#
day=$((24 * 60 * 60))
ctime=`/bin/date "+%s"`
ftime=`/usr/bin/stat -c "%Y" ${ABORT_FILE}`
if [ $? -eq 0 ]
then
	rm -f ${ABORT_FILE}
	if ((ctime - ftime < day)); then
		echo "## Abort signaled. Skipping balance ##" >> $LOGF
		exit 0
	fi
fi


convert() {
	encoded=$1
	echo $encoded | awk -v mult="1" \
                '{ if (match($1, /MiB$/)) {
                           sub(/MiB/, "", $1)
                   }
                   if (match($1, /GiB/)) {
                           sub(/GiB/, "", $1)
                           mult=1024
                   }
                   if (match($1, /TiB/)) {
                           sub(/TiB/, "", $1)
                           mult=1024*1024
                   }
                   print $1 * mult }'
}

rotate_log()
{
	sz=$(/usr/bin/stat -c "%s" $LOGF)
	if [ $sz -gt $ROTATE_THRESH ]
	then
		mv $LOGF ${LOGF}.1
		touch $LOGF
	fi
}

get_usages()
{
	check_cache=$1
	if [ "$check_cache" = "check_cache" ]
	then
		CACHED_TOT=0
		CACHED_USED=0
		CACHED_ALLOC=0
		CACHED_TARGET=0
		if [ -f $CACHE_FILE ]
		then
			set -- `cat $CACHE_FILE`
			CACHED_TOT=$1
			CACHED_USED=$2
			CACHED_ALLOC=$3
			CACHED_TARGET=$4
		fi
	fi

	show=`/sbin/btrfs fi show $ROOTFS 2>&1`
	set -- `echo "$show" | grep "bytes used"`
	Used=$7

	set -- `echo "$show" | grep "/dev/"`
	Tot=$4
	Alloc=$6
	MAXUSE=`convert $Tot`
	ALLOC=`convert $Alloc`

	if [ "$check_cache" = "check_cache" ]
	then
		/bin/echo -n "$Tot $Used $Alloc" > $CACHE_FILE
		if [ "$Alloc" = "$CACHED_ALLOC" -a "$Used" = "$CACHED_USED" ]
		then
			#
			# Return non-zero if nothing changed from last run.
			# These are rounded values to nearest tens of MB
			# (btrfs fi show output).
			#
			return 1
		fi
	fi
}

get_wasted()
{
	type=$1
	mixed=$2
	ratio=1
	get_usages

	set -- `/sbin/btrfs fi df $ROOTFS | grep "^$type"`
	[ "$2" = "DUP:" -o "$2" = "RAID1" -o "$2" = "RAID10" ] && ratio=2
	total=`echo $3 | awk -F'=' '{ print $2 }'`
	used=`echo $4 | awk -F'=' '{ print $2 }'`
	total=`convert $total`
	used=`convert $used`

	target_wasted_ratio="$TARGET_WASTED_RATIO"
	[ "$type" = "$TYPE_META" ] && target_wasted_ratio="$META_TARGET_WASTED_RATIO"
	unalloc=$(echo "$MAXUSE - $ALLOC" | bc -l)
	free=$(echo "($unalloc + ($total - $used)) * $ratio" | bc -l)
	echo "if (1 - ($unalloc / $free) > $target_wasted_ratio) { print \"1\"; } else { print \"0\" }" | bc -l
}

report_progress()
{
	echo "BALANCE_PROGRESS:$1" >> $LOGF
}

run_balance()
{
	filter=$1
	targ=$2
	fs=$3

	if [ -f ${ABORT_FILE} ]
	then
		rm -f ${ABORT_FILE}
		echo "## Balance aborted ##" >> $LOGF
		exit 27
	fi
	filt=`echo $filter | sed "s/TARGET/$targ/g"`
	/bin/date >> $LOGF
	/usr/bin/nice -n 10 /sbin/btrfs balance start -v ${filt} ${fs} >> $LOGF 2>&1
	/bin/date >> $LOGF
	DID_RUN_BALANCE=1

	#
	# After last operation, sleep for at least the commit interval to allow
	# btrfs to commit pending cached data to disk before potentially generating
	# new data in the next round.
	# This avoids quickly doing successive balance operations and increasing the
	# amount of data to be flushed.
	#
	/bin/sleep $FS_COMMIT_INTERVAL
}

get_usages "check_cache"
exit_code=$?

more_usage=`echo "if ($ALLOC / $MAXUSE > 0.85) { print \"1\" } else { print \"0\" }" | bc -l`
if ((more_usage == 1))
then
	do_meta_balance=1
	MAX_TARGET_THRESH1=70
	MAX_TARGET_THRESH2=75
fi
MAX_TARGET=$MAX_TARGET_THRESH1

# We start from empty block groups and advance to more full ones.
echo "#### Starting btrfs balance ####" >> $LOGF
date >> $LOGF

if [ $exit_code -ne 0 ]
then
	if [ "$CACHED_TARGET" = "$MAX_TARGET_THRESH1" ]
	then
		MAX_TARGET=$MAX_TARGET_THRESH2
		exit_code=0
	fi
fi

if [ $exit_code -eq 0 ]
then
	echo " $MAX_TARGET" >> $CACHE_FILE

	echo "#### Cleaning up free extents ####" >> $LOGF
	args="-musage=TARGET -dusage=TARGET"
	report_progress 0
	run_balance "$args" 0 $ROOTFS

	# if we can't rebalance with 0% target, then Something BAD.
	exit_code=$?

	#
	# Check for mixed block groups
	#
	dev=`cat /proc/mounts | awk '{ if ($2 == "/") { print $1; } }'`
	flags=`/sbin/btrfs-show-super $dev | awk '/^incompat_flags/ { print strtonum($2); }'`
	mixed=0
	if (( (flags & 4) == 4 ))
	then
		echo "#### Mixed block groups detected ####" >> $LOGF
		mixed=1
	fi

	args="-musage=TARGET"
	if (( mixed == 0 ))
	then
		echo "#### Balancing Metadata ... ####"  >> $LOGF
	else
		echo "#### Balancing Metadata and Data ... ####" >> $LOGF
		args="-musage=TARGET -dusage=TARGET"
	fi
else
	echo "No change in allocations from last rebalance. Skipping." >> $LOGF
fi

if [ $exit_code -eq 0 ]; then
	#
	# First balance metadata then data.
	#
	if (( mixed == 0 )); then
		usage_target=5
		failures=0
		count=0
		meta_free_wasted=`get_wasted "$TYPE_META" $mixed`
		while (( (meta_free_wasted > 0) && (count < MAX_REBALANCES) && (do_meta_balance == 1)))
		do
			report_progress $usage_target
			run_balance "$args" $usage_target $ROOTFS
			if [ $? -ne 0 ]; then
				exit_code=$?
				failures=$((failures + 1))
				if ((usage_target == 0))
				then
					echo "Metadata Balancing failed usage_target reached 0" >> $LOGF
					break
				fi
				if ((failures >= MAX_FAILURES))
				then
					echo "Too many rebalance failures: $MAX_FAILURES"
					break
				fi
				usage_target=$((usage_target - 1))
			else
				failures=0
				usage_target=$((usage_target + 5))
				[ $usage_target -gt $MAX_META_TARGET ] && break
			fi
			meta_free_wasted=`get_wasted "$TYPE_META" $mixed`
			count=$((count + 1))
		done
		meta_exit_code=$exit_code
		exit_code=0
		report_progress 100
		echo "#### DONE ####" >> $LOGF

		echo "#### Balancing Data ... ####"  >> $LOGF
		args="-dusage=TARGET"
	fi

	usage_target=5
	failures=0
	count=0
	data_free_wasted=`get_wasted "$TYPE_DATA" $mixed`
	while (( (data_free_wasted > 0) && (count < MAX_REBALANCES) ))
	do
		report_progress $usage_target
		run_balance "$args" $usage_target $ROOTFS
		if [ $? -ne 0 ]; then
			exit_code=$?
			failures=$((failures + 1))
			if ((usage_target == 0))
			then
				echo "Data Balancing failed usage_target reached 0" >> $LOGF
				break
			fi
			if ((failures >= MAX_FAILURES))
			then
				echo "Too many rebalance failures: $MAX_FAILURES"
				break
			fi
			usage_target=$((usage_target - 1))
		else
			failures=0
			usage_target=$((usage_target + 5))
			[ $usage_target -gt $MAX_TARGET ] && break
		fi
		data_free_wasted=`get_wasted "$TYPE_DATA" $mixed`
		count=$((count + 1))
	done
	report_progress 100
	echo "#### DONE ####" >> $LOGF
fi

/bin/date >> $LOGF
echo "Btrfs balance finished" >> $LOGF
[ $DID_RUN_BALANCE -eq 1 ] && /sbin/btrfs quota rescan $ROOTFS >> $LOGF 2>&1
echo "" >> $LOGF
rotate_log

exit $exit_code
