#!/bin/bash

echo -ne "Content-Type: text/html\r\n\r\n"

read -n "$CONTENT_LENGTH" POST_DATA

NAME=$(echo "$POST_DATA" | cut -d'=' -f2 | sed 's/+/ /g;s/%/\\x/g' | xargs -0 echo -e 2>/dev/null)

if [ -z "$NAME" ]; then
    NAME="Stranger"
fi

echo "<!DOCTYPE html>"
echo "<html>"
echo "<head><style>body { font-family: sans-serif; background: #2c3e50; color: white; text-align: center; padding: 50px; }</style></head>"
echo "<body>"
echo "<h1>Hello, $NAME!</h1>"
echo "<br><a href='/' style='color: #48dbfb;'>Back to home</a>"
echo "</body>"
echo "</html>"