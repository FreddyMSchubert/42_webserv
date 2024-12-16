import os
import logging

def setup_logger(log_file: str):
	"""Configure the logger to write to a specified log file."""
	logging.basicConfig(
		filename=log_file,
		level=logging.INFO,
		format='%(asctime)s - %(levelname)s - %(message)s'
	)

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
