#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, os.path
from pwd import getpwnam
import re
import subprocess
import shutil
import cPickle
import urllib2
import socket

hostname = socket.getfqdn()
apacheroot = "/etc/httpd/conf"

APIPASS = "W68p20YST5Iv6KGG"

authhandler = urllib2.HTTPBasicAuthHandler()
authhandler.add_password(realm="Pysk", uri="https://%s/" % (hostname,), user="pysk", passwd=APIPASS)

opener = urllib2.build_opener(authhandler)
urllib2.install_opener(opener)

print "Getting config for %s ..." % (hostname,)
vhosts = cPickle.load(urllib2.urlopen("https://%s/api/v0/vz/apache/" % (hostname,)))
validDomain = re.compile("([a-zA-Z0-9-]+\.?)+")

if os.path.exists("%s/sites-available/" % (apacheroot,)):
	shutil.rmtree("%s/sites-available/" % (apacheroot,))
if os.path.exists("%s/sites-enabled/" % (apacheroot,)):
	shutil.rmtree("%s/sites-enabled/" % (apacheroot,))
os.mkdir("%s/sites-available/" % (apacheroot,), 0755)
os.mkdir("%s/sites-enabled/" % (apacheroot,), 0755)

portsconf = open("%s/ports.conf" % (apacheroot,), "w")
portsconf.write("Listen 127.0.0.1:80\n")
portsconf.write("NameVirtualHost 127.0.0.1:80\n")
portsconf.close()

f = open("%s/sites-available/default" % (apacheroot,), "w")
f.write("""
<VirtualHost 127.0.0.1:80>
    ServerAdmin philipp@igowo.de
    DocumentRoot /srv/http/default/htdocs/

    #CustomLog /dev/null common env=DOES_NOT_EXIST
    DirectoryIndex index.html index.htm

    <Directory /srv/http/default/htdocs/>
        Options Indexes FollowSymLinks MultiViews
        AllowOverride None
        Order allow,deny
        allow from all
    </Directory>

    <Location /server-status>
        SetHandler server-status
        Order Deny,Allow
        Deny from all
        Allow from 127.0.0.1
    </Location>
</VirtualHost>
""")
f.close()

# Create default vhost directory
if not os.path.exists("/srv/http/default/htdocs/"):
	os.makedirs("/srv/http/default/htdocs/", 0755)
os.chown("/srv/http/default/htdocs/", 0, 0)
subprocess.call(["/opt/pysk/tools/apache/a2ensite", "default"])

for key in vhosts:
    (value, username, htdocs_dir) = vhosts[key]

    if validDomain.match(key).group(0) != key:
        raise Exception("Invalid domain name = '%s'!" % (key,))

    print "Generating virtual host '%s' ..." % (key,)
    outpath = "%s/sites-available/%s" % (apacheroot, key,)
    f = open(outpath, "w")
    f.writelines(value)
    f.close()

    # Fixup htdocs dir
    uid = getpwnam(username).pw_uid
    gid = 100

    for dir in [os.path.realpath(htdocs_dir), os.path.realpath(os.path.join(htdocs_dir, "../"))]:
        print "Fixing %s ..." % (dir,)

        if not os.path.exists(dir):
            print "WARNING: htdocs dir does not exist: %s" % (dir)
            os.makedirs(dir, 0755)

        statinfo = os.stat(dir)
        if not statinfo.st_uid == uid:
            print "WARNING: htdocs dir has wrong owner: %i != %i" % (uid, statinfo.st_uid)
            os.chown(dir, uid, -1)
        if not statinfo.st_gid == gid:
            print "WARNING: htdocs dir has wrong group: %i != %i" % (gid, statinfo.st_gid)
            os.chown(dir, -1, gid)

    subprocess.call(["/opt/pysk/tools/apache/a2ensite", key])

subprocess.call(["/usr/sbin/apachectl", "-t"])

