from __future__ import print_function

import cgi
import json
import athletemodel
import yate

import sys

athletes = athletemodel.get_from_store()
form_data = cgi.FieldStorage()
athlete_name = form_data['which_athlete'].value
print("generate_data: " + json.dumps(athletes[athlete_name]), sys.stderr)

print(yate.start_response('application/json'))
print(json.dumps(athletes[athlete_name].as_dict))
