#!/usr/bin/env python3
import os
import cgi
import cgitb
from http import cookies
from common import load_sessions, find_user

cgitb.enable()

form = cgi.FieldStorage()
data = form.getvalue("data", "").strip()

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
			# Save data to a file named after the user's email
			data_dir = os.path.join(os.path.dirname(__file__), "..", "data")
			os.makedirs(data_dir, exist_ok=True)
			user_data_file = os.path.join(data_dir, f"{email}.txt")
			try:
				with open(user_data_file, "a") as f:
					f.write(f"{data}\n")
				print("<html><body style='font-family:serif;'>")
				print(f"<h1>Data Saved Successfully, {name}! 🎉 </h1>")
				print("<p><a href='welcome.py'>Back to Welcome Page</a></p>")
				print("</body></html>")
			except Exception as e:
				print("<html><body style='font-family:serif;'>")
				print(f"<h1>Error Saving Data: {e} 🛑 </h1>")
				print("<p><a href='save_data.html'>Try Again</a></p>")
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
