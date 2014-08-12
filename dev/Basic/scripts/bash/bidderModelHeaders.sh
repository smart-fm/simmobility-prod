#!/bin/bash 

echo " "
echo "A utility to add headers to the Bidder model output files"
echo " "
if [ $# -ne 2 ]; then	
	echo "USAGE: ./bidderModelHeaders.sh [bids/expectations] [path to file]"
	echo "Example: ./bidderModelHeaders.sh bids ../../Debug/bids.csv"
	echo " "
else
	if [ $1 == "expectations" ]; then
		echo "Adding headers to the expectations output file"
		sed -i '1 i \bid_timestamp, day_to_apply, seller_id, unit_id, hedonic_price, asking_price, target_price' $2
		echo " "
	elif [ $1 == "bids" ]; then
		echo "Adding headers to the bids output file"
		sed -i '1 i \bid_timestamp ,seller_id, bidder_id, unit_id, bidder wp (total-price), speculation (discount), asking_uprice (per sqft), floor area (sqft), type_id, target_uprice (per sqft), bid_value (total-price), bids_counter (daily), status(1 = ACCEPTED)' $2
		echo " "
	else
		echo "Unknown usage. Run \"./bidderModelHeaders.sh\" for sample usage help."
	fi
fi

echo "Script will now exit."
