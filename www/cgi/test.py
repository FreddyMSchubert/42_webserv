import os
import logging

print("hiiiiiii", flush=True)

print("Current Working Directory:", os.getcwd())
try:
	with open("/Users/fschuber/42_webserv/test_file.log", "w") as f:
		f.write("File write test successful.")
	print("File creation successful.")
except Exception as e:
	print(f"Error creating file: {e}")

print("Environment Variables:")
for key, value in os.environ.items():
	print(f"{key}={value}")

def setup_logger(log_file: str):
	"""Configure the logger to write to a specified log file."""
	try:
		logging.basicConfig(
			filename=log_file,
			level=logging.INFO,
			format='%(asctime)s - %(levelname)s - %(message)s'
		)
		logging.info("Logger setup successful.")
	except Exception as e:
		print(f"Error setting up logger: {e}")

def log_environment_variables():
	"""Retrieve and log all environment variables."""
	env_vars = os.environ
	for key, value in env_vars.items():
		logging.info(f'{key}={value}')

def main():
	log_file = 'env_variables.log'
	setup_logger(log_file)
	logging.info('Starting to log environment variables.')
	log_environment_variables()
	logging.info('Finished logging environment variables.')

if __name__ == '__main__':
	main()
