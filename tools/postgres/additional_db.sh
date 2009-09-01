#!/bin/bash

set -e
set -u

DBNAME=$1
DBMAINUSER=$2

psql -U postgres template1 -f - <<EOT
CREATE ROLE $DBNAME NOSUPERUSER NOCREATEDB NOCREATEROLE NOINHERIT NOLOGIN;
GRANT $DBNAME TO $DBMAINUSER;
CREATE DATABASE $DBNAME WITH OWNER=$DBMAINUSER;
EOT

psql -U postgres $DBNAME -f - <<EOT
GRANT ALL ON SCHEMA public TO $DBMAINUSER WITH GRANT OPTION;
EOT

