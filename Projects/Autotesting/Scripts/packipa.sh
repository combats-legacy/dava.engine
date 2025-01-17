# !/bin/bash

# Copyright (c) 2011 Float Mobile Learning
# http://www.floatlearning.com/
#
# Permission is hereby granted, free of charge, to any person obtaining 
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included 
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Please let us know about any improvements you make to this script!
# ./sign source identity -p "path/to/profile" -e "path/to/entitlements" target

if [ $# -lt 2 ]; then
	echo "usage: $0 source target" >&2
	exit 1
fi

#if [ $# -lt 3 ]; then
#	echo "usage: $0 source identity [-p provisioning] [-e entitlements] target" >&2
#	exit 1
#fi

ORIGINAL_FILE="$1"
#CERTIFICATE="$2"
#NEW_PROVISION=
#ENTITLEMENTS=

#OPTIND=3
#while getopts p:e: opt; do
#	case $opt in
#		p)
#			NEW_PROVISION="$OPTARG"
#			echo "Specified provisioning profile: $NEW_PROVISION" >&2
#			;;
#		e)
#			ENTITLEMENTS="$OPTARG"
#			echo "Specified signing entitlements: $ENTITLEMENTS" >&2
#			;;
#		\?)
#			echo "Invalid option: -$OPTARG" >&2
#			exit 1
#			;;
#		:)
#			echo "Option -$OPTARG requires an argument." >&2
#			exit 1
#			;;
#	esac
#done

#shift $((OPTIND-1))

#NEW_FILE="$1"
NEW_FILE="$2"

# Check if the supplied file is an ipa or an app file
#if [ "${ORIGINAL_FILE#*.}" = "ipa" ]
#	then
#		# Unzip the old ipa quietly
#		unzip -q "$ORIGINAL_FILE" -d temp
#el
if [ "${ORIGINAL_FILE#*.}" = "app" ]
	then
		# Copy the app file into an ipa-like structure
		mkdir -p "temp/Payload"
		cp -Rf "$ORIGINAL_FILE" "temp/Payload/$ORIGINAL_FILE"
else
    echo "Error: Only can pack .app files." >&2
#	echo "Error: Only can resign .app files and .ipa files." >&2
	exit
fi

# Set the app name
# The app name is the only file within the Payload directory
APP_NAME=$(ls temp/Payload/)
echo "APP_NAME=$APP_NAME" >&2

# Replace the embedded mobile provisioning profile
#if [ "$NEW_PROVISION" != "" ]; then
#	echo "Adding the new provision: $NEW_PROVISION"
#	cp "$NEW_PROVISION" "temp/Payload/$APP_NAME/embedded.mobileprovision"
#fi

# Resign the application
#echo "Resigning application using certificate: $CERTIFICATE" >&2
#if [ "$ENTITLEMENTS" != "" ]; then
#	echo "Using Entitlements: $ENTITLEMENTS" >&2
#	/usr/bin/codesign -f -s "$CERTIFICATE" --entitlements="$ENTITLEMENTS" --resource-rules="temp/Payload/$APP_NAME/ResourceRules.plist" "temp/Payload/$APP_NAME"
#else
#	/usr/bin/codesign -f -s "$CERTIFICATE" --resource-rules="temp/Payload/$APP_NAME/ResourceRules.plist" "temp/Payload/$APP_NAME"
#fi

# Repackage quietly
echo "Packaging as $NEW_FILE"

# Zip up the contents of the temp folder
# Navigate to the temporary directory (sending the output to null)
# Zip all the contents, saving the zip file in the above directory
# Navigate back to the orignating directory (sending the output to null)
pushd temp > /dev/null
zip -qry ../temp.ipa *
popd > /dev/null

# Move the resulting ipa to the target destination
mv temp.ipa "$NEW_FILE"

# Remove the temp directory
rm -rf "temp"