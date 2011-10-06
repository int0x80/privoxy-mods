#!/usr/bin/env python
# -----------------------------------------------------------
# grabs list of ad servers from pgl.yoyo.org and 
# sets up privoxy to block the enumerated domains
# *note*: be sure to update variables down there ;]
#
# usage: python privbloxy.py
# -----------------------------------------------------------

import urllib2 

# -----------------------------------------------------------
# re-write privoxy config, but add in the ads actionfile
# -----------------------------------------------------------
def add_ads_action(dir, lines, user_line):
    line_num = 0
    config = open(dir + "/config.txt", "w")
    for line in lines:
        config.write(line)        
        if line_num == user_line:
            config.write("actionsfile ads.action       # Blocking ad servers enumerated on pgl.yoyo.org\n")
        line_num += 1
    config.close()

    
# -----------------------------------------------------------
# determine whether ads actionfile entry exists or update
# -----------------------------------------------------------
def check_config(dir):
    ads_entry = False
    curr_line = 0
    user_line = 0
    
    config_lines = open(privoxy_dir + "/config.txt", "r").readlines()
    for line in config_lines:    
        # -----------------------------------------------------------
        # check for ads action file; bail if present
        # -----------------------------------------------------------
        if line.startswith("actionsfile ads.action"):
            ads_entry = True
            break
        # -----------------------------------------------------------
        # line after user actions was not ads actions
        # -----------------------------------------------------------
        if user_line > 0:
            add_ads_action(privoxy_dir, config_lines, user_line)
            break
        # -----------------------------------------------------------
        # note location of user actions
        # -----------------------------------------------------------
        if line.startswith("actionsfile user.action"):
            user_line = curr_line
        curr_line += 1


# -----------------------------------------------------------
# parse and set actionfile contents
# -----------------------------------------------------------
def parse_adfile(adfile):
    adlist = "{+block{Ad domains from pgl.yoyo.org} +handle-as-empty-document}\n"
    lines = adfile.readlines()
    for line in lines:
        # -----------------------------------------------------------
        # skip comment lines starting with '#' and blank lines
        # -----------------------------------------------------------
        if line.startswith("#") or not line.strip():
            continue        
        # -----------------------------------------------------------
        # domains are separated by commas
        # -----------------------------------------------------------        
        adlist = adlist + line.replace(",", "\n")
        
    return adlist


# -----------------------------------------------------------
# download latest ads list, parse, and write to file
# -----------------------------------------------------------
def set_adlist(dir, url):
    adfile = urllib2.urlopen(url)
    adlist = parse_adfile(adfile)
    actionfile = open(dir + "/ads.action", "w")
    actionfile.write(adlist)
    actionfile.close()




# -----------------------------------------------------------
# configuration stuffs, adjust accordingly
# -----------------------------------------------------------
privoxy_dir = "C:/Program Files/Privoxy"
adlist_url = "http://pgl.yoyo.org/adservers/serverlist.php?hostformat=one-line&showintro=0&mimetype=plaintext"

check_config(privoxy_dir)
set_adlist(privoxy_dir, adlist_url)