#!/usr/bin/env python3
import datetime

print("Content-Type: text/html\r\n\r\n")

now = datetime.datetime.now()

formatted_date = now.strftime("%d/%m/%Y")
formatted_time = now.strftime("%H:%M:%S")

#print("<html>")
#print("<head><title>CGI Timestamp Test</title></head>")
#print("<body>")
#print("<h1>Real time testing</h1>")
#print(f"<p>Today is <strong>{formatted_date}</strong> {formatted_time}</p>")
#print("<p><i>Refresh the page to use again the CGI</i></p>")
#print("</body>")
#print("</html>")

print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("    <meta charset='UTF-8'>")
print("    <title>CGI Timestamp Test</title>")
print("    ")
print("    <link href='https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700;900&display=swap' rel='stylesheet'>")
print("    <style>")
print("        body {")
print("            font-family: 'Montserrat', sans-serif;")
print("            background-color: #2c3e50;")
print("            color: #ffffff;")
print("            padding: 30px;")
print("        }")
print("        strong {")
print("            color: #f1c40f;")
print("        }")
print("    </style>")
print("</head>")
print("<body>")

print("    <h1>Real time testing</h1>")
print(f"    <p>Today is <strong>{formatted_date}</strong> {formatted_time}</p>")
print("    <p><i>Refresh the page to use again the CGI, the timestamp will be updated</i></p>")

print("<div style='margin-top: 40px;'>")
print("  <a href='/' style='padding: 12px 20px; background-color: #0e6251; color: #e0e0e0; text-decoration: none; border-radius: 5px; font-weight: bold; border: 1px solid #117a65;'>Back to Home</a>")
print("</div>")

print("</body>")
print("</html>")