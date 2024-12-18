#!/usr/bin/env python3
import cgi
import cgitb
import uuid
from common import find_user, save_session

cgitb.enable()

form = cgi.FieldStorage()
email = form.getvalue("email", "")
password = form.getvalue("password", "")

print("Content-Type: text/html")

if not (email and password):
	print("\n")
	print("<html><body style='font-family:serif;'>")
	print("<h1>Missing Credentials (ಠ_ಠ)</h1>")
	print("<p><a href='../index.html'>Back</a></p>")
	print("</body></html>")
else:
	user = find_user(email)
	if user and user[2] == password:
		# Login success
		session_id = str(uuid.uuid4())
		save_session(session_id, email)
		# Set cookie
		print(f"Set-Cookie: session_id={session_id}; Path=/; HttpOnly")
		print("\n")
		print("<html><body style='font-family:serif;'>")
		print("<h1>Login Successful! (ﾉ◕ヮ◕)ﾉ*:･ﾟ✧</h1>")
		print("<p><a href='../cgi-bin/welcome.py'>Go to your Welcome Page</a></p>")
		print("</body></html>")
	else:
		# Fail
		print("\n")
		print("<html><body style='font-family:serif;'>")
		print("<h1>Incorrect Email or Password (╯°□°)╯︵ ┻━┻</h1>")
		print("<p><a href='../index.html'>Try Again</a></p>")
		print("</body></html>")
