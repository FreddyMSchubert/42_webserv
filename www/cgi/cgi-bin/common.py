#!/usr/bin/env python3
import os

USER_FILE = os.path.join(os.path.dirname(__file__), "..", "users.txt")
SESSION_FILE = os.path.join(os.path.dirname(__file__), "..", "sessions.txt")

def load_users():
	users = []
	if os.path.exists(USER_FILE):
		with open(USER_FILE, "r") as f:
			for line in f:
				line = line.strip()
				if line:
					name, email, password = line.split("||")
					users.append((name, email, password))
	return users

def save_user(name, email, password):
	with open(USER_FILE, "a") as f:
		f.write(f"{name}||{email}||{password}\n")

def find_user(email):
	for (name, em, pw) in load_users():
		if em == email:
			return (name, em, pw)
	return None

def load_sessions():
	sessions = {}
	if os.path.exists(SESSION_FILE):
		with open(SESSION_FILE, "r") as f:
			for line in f:
				line = line.strip()
				if line:
					session_id, email = line.split("||")
					sessions[session_id] = email
	return sessions

def save_session(session_id, email):
	with open(SESSION_FILE, "a") as f:
		f.write(f"{session_id}||{email}\n")

def remove_session(session_id):
	sessions = load_sessions()
	if session_id in sessions:
		del sessions[session_id]
	# rewrite sessions
	with open(SESSION_FILE, "w") as f:
		for sid, em in sessions.items():
			f.write(f"{sid}||{em}\n")

