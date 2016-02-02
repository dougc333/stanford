/*
 * File: errors.h
 * --------------
 * This file defines a set of error strings for use in producing the
 * same wording/spelling as our error messages and thus enabling diff
 * to easily compare your output against ours. Please be sure to
 * use these string constants when reporting errors.
 */

#ifndef _H_errors
#define _H_errors

// Errors used by preprocessor
static const char *err_unterm_comment = "Input ends with unterminated comment";
static const char *err_inval_dir = "Invalid # directive";

// Errors used by scanner
static const char *err_long_ident = "Identifier too long: \"%s\"";
static const char *err_unterm_string = "Unterminated string constant: %s";
static const char *err_unrecog_char = "Unrecognized char: \'%c\'";
  

#endif
