#!/bin/bash

echo "Content-Type: text/html"
echo ""

echo "<html>"
echo "<body>"

echo "<h1>CGI Debug</h1>"

echo "<h2>Current directory:</h2>"
echo "<pre>"
pwd
echo "</pre>"

echo "<h2>Environment variables:</h2>"
echo "<pre>"

env | sort

echo "</pre>"

echo "</body>"
echo "</html>"