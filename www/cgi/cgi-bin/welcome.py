#!/usr/bin/env python3
import os
import cgi
import cgitb
from http import cookies
from common import load_sessions, find_user

cgitb.enable()

print("Content-Type: text/html")

# Parse cookies
cookie = cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
session_id = cookie.get("session_id")

print("\n")  # End headers

if not session_id:
	# No session id found
	print("<html><body style='font-family:serif;'>")
	print("<h1>No Session Found (っ °Д °;)っ</h1>")
	print("<p><a href='../index.html'>Back Home</a></p>")
	print("</body></html>")
else:
	session_id = session_id.value
	sessions = load_sessions()
	if session_id in sessions:
		email = sessions[session_id]
		user = find_user(email)
		if user:
			name = user[0]
			print("<html><body style='font-family:serif;'>")
			print(f"<h1>Greetings, {name}! 🥂</h1>")
			print("<p>You are logged in. (ง'̀-'́)ง</p>")
			print("<p><a href='logout.py'>Logout</a></p>")
			print("</body></html>")
		else:
			print("<html><body style='font-family:serif;'>")
			print("<h1>Oops, could not find your user info! (⊙_⊙;)</h1>")
			print("<p><a href='../index.html'>Back Home</a></p>")
			print("</body></html>")
	else:
		print("<html><body style='font-family:serif;'>")
		print("<h1>Invalid Session, old chap! (ಥ_ಥ)</h1>")
		print("<p><a href='../index.html'>Back Home</a></p>")
		print("</body></html>")
