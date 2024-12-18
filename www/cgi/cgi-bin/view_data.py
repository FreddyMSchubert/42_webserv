#!/usr/bin/env python3
import os
import cgi
import cgitb
import html  # Import the html module for escaping
from http import cookies
from common import load_sessions, find_user

cgitb.enable()

print("Content-Type: text/html; charset=utf-8")

# Parse cookies
cookie = cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
session_id = cookie.get("session_id")

print("\n")  # End headers

if not session_id:
	# No session id found
	print("<html><body style='font-family:serif;'>")
	print("<h1>No Session Found! (っ °Д °;)っ </h1>")
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
			# Read data from user's file
			data_dir = os.path.join(os.path.dirname(__file__), "..", "data")
			user_data_file = os.path.join(data_dir, f"{email}.txt")
			print("<html><body style='font-family:serif;'>")
			print(f"<h1>Your Saved Data, {html.escape(name)}! 📂 </h1>")
			if os.path.exists(user_data_file):
				try:
					with open(user_data_file, "r") as f:
						data_lines = f.readlines()
					if data_lines:
						print("<ul>")
						for line in data_lines:
							safe_line = html.escape(line.strip())
							print(f"<li>{safe_line}</li>")
						print("</ul>")
					else:
						print("<p>You haven’t saved any data yet! Start now! 🚀</p>")
				except Exception as e:
					# Escape the error message to prevent potential XSS
					safe_error = html.escape(str(e))
					print(f"<p>Error reading data: {safe_error} 🛑 </p>")
			else:
				print("<p>No data found. Maybe it’s hiding? 🕵️‍♂️🔍</p>")
			print("<p><a href='welcome.py'>Back to Welcome Page</a></p>")
			print("</body></html>")
		else:
			print("<html><body style='font-family:serif;'>")
			print("<h1>Oops, could not find your user info! (⊙_⊙;) </h1>")
			print("<p><a href='../index.html'>Back Home</a></p>")
			print("</body></html>")
	else:
		print("<html><body style='font-family:serif;'>")
		print("<h1>Invalid Session, old chap! (ಥ_ಥ) </h1>")
		print("<p><a href='../index.html'>Back Home</a></p>")
		print("</body></html>")
