if [ "$EUID" -ne 0 ]; then
  echo "This script must be run with sudo."
  exit 1
fi

gcc -v -o /usr/bin/edim edim.c
