import sys
from ua_parser import user_agent_parser as uap
import traceback

def write_str(s):
    sys.stdout.buffer.write(bytearray(s), 'ascii')

for line in sys.stdin:
    try: 
        ua = uap.Parse(line)
        browser = ua.get("user_agent",{})
        os = ua.get("os",{})
        device = ua.get("device",{})
        fields = [
	    browser.get("family"     , ""),
	    browser.get("major"      , ""),
	    browser.get("minor"      , ""),
	    browser.get("patch"      , ""),
	    browser.get("patch_minor", ""),
	    os     .get("family"     , ""),
	    os     .get("major"      , ""),
	    os     .get("minor"      , ""),
	    os     .get("patch"      , ""),
	    os     .get("patch_minor", ""),
	    device .get("family"     , ""),
	    device .get("brand"      , ""),
	    device .get("model"      , ""),
        ]
        fields = ["" if f is None else f for f in fields]
        sys.stdout.write("\t".join(fields))
        sys.stdout.write('\n')
    except Exception as e:
        print(e)
        sys.stdout.flush()
        traceback.print_exc()
        print(fields)
        print(str(ua))
        exit(1)
        
sys.stdout.flush()
sys.stdout.close()
