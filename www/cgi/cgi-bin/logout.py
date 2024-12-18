#!/usr/bin/env python3
import os
import cgi
import cgitb
from http import cookies
from common import remove_session

cgitb.enable()

print("Content-Type: text/html")

cookie_str = os.environ.get("HTTP_COOKIE", "")
cookie = cookies.SimpleCookie(cookie_str)
session_id = cookie.get("session_id")

print("\n")  # Headers done

if session_id:
	remove_session(session_id.value)
	# Invalidate cookie
	print("<html><body style='font-family:serif;'>")
	print("<h1>You've been logged out. ( ´･･)ﾉ(._.`)</h1>")
	print("<p><a href='../index.html'>Back Home</a></p>")
	print("</body></html>")
else:
	print("<html><body style='font-family:serif;'>")
	print("<h1>No session to log out from, dear fellow! (・_・;)</h1>")
	print("<p><a href='../index.html'>Back Home</a></p>")
	print("</body></html>")
