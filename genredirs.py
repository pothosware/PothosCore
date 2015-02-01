import sys
import os

TMPL = """\
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <meta http-equiv="refresh" content="0; url=./doxygen/latest/%s" />
    </head>
    <body>
        <b>Doxygen generated documentation moved! <a href="./doxygen/latest/%s">Redirecting...</a></b>
    </body>
</html>
"""

if __name__ == '__main__':
    for redir in open('redirects.txt').readlines():
        redir = redir.strip()
        open(redir, 'w').write(TMPL%(redir,redir))
