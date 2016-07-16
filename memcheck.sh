#!/bin/bash

if [ "${1}" == "-h" ] || [ "${1}" == "--help" ] || [ $# -lt 1 ] ; then
    echo -n "Memory checker - Helper for running a command with "
    echo        "valgrind."
    echo    ""
    echo    "Environmental variables:"
    echo -n "  VALGRIND: Path to valgrind binary. Defaults to "
    echo        "/opt/valgrind/bin/valgrind"
    echo -n "  LOG_FILE: File where valgrind's logs are written. "
    echo        "Defaults to ./valg.log"
    echo    "  QUIET   : If \"true\", redirects output to /dev/null."
    echo    "  VERBOSE : If \"true\", sets valgrind's verbosity."
    echo    ""
    exit 0
fi

# Set the base command (i.e., set valgrind's path)
if [ -z "${VALGRIND}" ]; then
    CMD=/opt/valgrind/bin/valgrind
else
    CMD=${VALGRIND}
fi

# Set the log filename
if [ -z "${LOG_FILE}" ]; then
    CMD=`echo "${CMD} --log-file=valg.log"`
else
    CMD=`echo "${CMD} --log-file=${LOG_FILE}"`
fi

# Set valgrind's verbosity
if [ "${VERBOSE}" == "true" ]; then
    CMD=`echo "${CMD} -v"`
fi

# Add all leak checks
CMD=`echo "${CMD} --leak-check=full --show-leak-kinds=all"`

# Append the command to be executed
CMD=`echo "${CMD} ${@}"`

# Run the command
if [ "${QUIET}" == "true" ]; then
    ${CMD} > /dev/null
else
    ${CMD}
fi

