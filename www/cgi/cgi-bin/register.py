#!/usr/bin/env python3
import cgi
import cgitb
from common import save_user, find_user

cgitb.enable()

print("Content-Type: text/html; charset=utf-8\n")

form = cgi.FieldStorage()
name = form.getvalue("name", "")
email = form.getvalue("email", "")
password = form.getvalue("password", "")

if not (name and email and password):
	print("<html><body style='font-family:serif;'>")
	print("<h1>Missing fields! (ಠ_ಠ)</h1>")
	print("<p><a href='../index.html'>Back</a></p>")
	print("</body></html>")
else:
	# Check if user already exists
	existing_user = find_user(email)
	if existing_user:
		print("<html><body style='font-family:serif;'>")
		print("<h1>Oh dear, that email is already registered! (◣_◢)</h1>")
		print("<p><a href='../index.html'>Back to Home</a></p>")
		print("</body></html>")
	else:
		save_user(name, email, password)
		print("<html><body style='font-family:serif;'>")
		print("<h1>Registration Successful! ヾ(⌐■_■)ノ♪</h1>")
		print("<p>You may now <a href='../login.html'>log in</a>.</p>")
		print("</body></html>")
