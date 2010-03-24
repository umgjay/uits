#!/bin/sh

# runUITS.sh
# UITS_Tool - Shell script for testing
#
# Created by Chris Angelli on 1/26/10.
# Copyright 2010 Universal Music Group. All rights reserved.


# ./UITS_Tool help

# ./UITS_Tool verify \
# --verbose \
# --audio test/test_payload.mp3 \
# --pub test/pubRSA2048.pem 


./UITS_Tool create \
--verbose \
--audio test/test_audio.mp3 \
--xsd test/uits.xsd \
--uits test/test_payload.mp3 \
--embed \
--algorithm RSA2048 \
--priv  test/privateRSA2048.pem \
--pub test/pubRSA2048.pem \
--pubID "33dce5a4f8b67303a290dc5145037569ca38036d" \
--nonce "QgYnkgYS" \
--Distributor "A Distributor" \
--ProductID "My product" \
--AssetID "ES1700800500" \
--AssetID_type "ISRC" \
--TID "Transaction ID" \
--UID "User ID" \
--URL "http://www.umusic.com" \
--PA "explicit" \
--Copyright "allrightsreserved" \
--Extra "extra content" \
--Extra_type "blah" \
--Time "2008-08-30T13:15:04Z" 
