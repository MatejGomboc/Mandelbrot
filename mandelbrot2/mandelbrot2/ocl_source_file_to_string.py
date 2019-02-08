#!/usr/bin/python

# Copyright (C) 2019 Matej Gomboc https://github.com/MatejGomboc
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.

import os, sys, argparse, re

def comment_remover(text):
	def replacer(match):
		s = match.group(0)
		if s.startswith('/'):
			return " " # note: a space and not an empty string
		else:
			return s
	pattern = re.compile(
		r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
		re.DOTALL | re.MULTILINE
	)
	return re.sub(pattern, replacer, text)


def empty_string_remover(text):
	new_text = text
	new_text_prev = ""
	while (new_text != new_text_prev):
		new_text_prev = new_text
		new_text = new_text.replace(" \"", "\"")
		new_text = new_text.replace("\t\"", "\"")
		new_text = new_text.replace(" \\n\"", "\\n\"")
		new_text = new_text.replace("\t\\n\"", "\\n\"")
	return new_text


def empty_line_remover(text):
	new_text = text
	new_text_prev = ""
	while (new_text != new_text_prev):
		new_text_prev = new_text
		new_text = new_text.replace("\n\n", "\n") # remove empty lines
	return new_text


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-v", "--verbose", help="increase output verbosity", action="store_true")
	parser.add_argument("-c", "--compact", help="remove all comments and empty lines", action="store_true")
	parser.add_argument("inputFileName", help="name of input OpenCL file with source", type=str)
	parser.add_argument("outputFileName", help="name of output C++ file with source as char string", type=str)
	args = parser.parse_args()

	if not args.inputFileName.lower().endswith(".cl"):
		sys.exit("invalid input file extension: " + args.inputFileName)
	if not args.outputFileName.lower().endswith(".h"):
		sys.exit("invalid output file extension: " + args.outputFileName)

	# preform clean
	if os.path.exists(args.outputFileName):
		os.remove(args.outputFileName)

	if args.verbose:
		print("input file: " + args.inputFileName)
		print("output file: " + args.outputFileName)

	try:
		stringSource = open(args.inputFileName, 'r').read()
	except FileNotFoundError:
		sys.exit("input file not found: " + args.inputFileName)

	if args.verbose:
		print("\noriginal OpenCL source:")
		print(stringSource)

	if args.compact:
		stringSource = comment_remover(stringSource) # remove all comments

	stringSource = stringSource.replace("\\", "\\\\") # for multi-line strings
	stringSource = stringSource.replace("\"", "\\\"") # for strings
	stringSource = stringSource.replace("\'", "\\\'") # for chars
	stringSource = stringSource.replace("\\\a", "\\\\\\\a") # if '\a' inside a string
	stringSource = stringSource.replace("\\\b", "\\\\\\\b") # if '\b' inside a string
	stringSource = stringSource.replace("\\\f", "\\\\\\\f") # if '\f' inside a string
	stringSource = stringSource.replace("\\\r", "\\\\\\\r") # if '\r' inside a string
	stringSource = stringSource.replace("\\\t", "\\\\\\\t") # if '\t' inside a string
	stringSource = stringSource.replace("\\\v", "\\\\\\\v") # if '\v' inside a string
	stringSource = stringSource.replace("\\\n", "\\\\\\\n") # if '\n' inside a string
	stringSource = stringSource.replace("\n", "\\n\"\n\"") # for correct ending of new multi-line string

	stringSource = "const char* " + args.inputFileName[:-3].replace("-", "_") + "_ocl_source = \n\"" + stringSource

	stringSource = empty_string_remover(stringSource) # remove redundant spaces and tabs
	stringSource = stringSource.replace("\"\\n\"", "") # remove empty multi-line strings
	stringSource = stringSource.replace("\"\"", "") # remove empty multi-line strings

	if args.compact:
		stringSource = empty_line_remover(stringSource) # remove empty lines

	stringSource = stringSource.replace("\t", "    ") # replace tabs with 4 spaces

	stringSource = stringSource + "\\0" + "\"\n;\n" # zero-terminate string

	if args.verbose:
		print("source as C++ char string:")
		print(stringSource)

	open(args.outputFileName, "w").write(stringSource)
