#!/bin/bash

badges=$(./tools/webusb_list_badges.py --flat)

# Check number.
if [ "$badges" = "none" ]; then
    echo "No badges to flash."
    exit 1
else
    echo "Installing to $(echo $badges | wc -w) badge(s)"
fi

# Start tasks.
for dev in $badges; do
    python3 tools/webusb_push.py "Spwn" build/main.bin --run --address=$dev &
done

function abort() {
    kill -9 $(jobs -lp)
    echo "Aborting!"
}

# Cancel handler.
trap ctrl_c abort

wait

echo "Done!"
