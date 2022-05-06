#!/bin/sh

WAIT_METHOD="route"
WAIT_ADDRESS="default"
WAIT_TIMEOUT="30"

[ -f /etc/default/networking ] && . /etc/default/networking

case "$WAIT_METHOD" in
route)
	(timeout "$WAIT_TIMEOUT" ip mon r & ip -4 r s; ip -6 r s) | grep -q "^$WAIT_ADDRESS\>"
	;;

ping)
	ping -q -c 1 -w "WAIT_TIMEOUT" "$WAIT_ADDRESS" >/dev/null
	;;

ping6)
	ping6 -q -c 1 -w "WAIT_TIMEOUT" "$WAIT_ADDRESS" >/dev/null
	;;

no|none)
	exit 0
	;;

*)
	echo "Unknown wait method $WAITONLINE"
	exit 1
	;;
esac
