class AthleteList(list):
    def __init__(self, a_name, a_dob=None, a_times=[]):
        list.__init__([])
        self.name = a_name
        self.dob = a_dob
        self.extend(a_times)
    @property
    def top3(self):
        return (sorted(set([sanitize(t) for t in self]))[0:3])
    @property
    def as_dict(self):
        return {'Name': self.name, 'DOB': self.dob, 'Top3': self.top3}


def sanitize(time_string):
    if '-' in time_string:
        splitter = '-'
    elif ':' in time_string:
        splitter = ':'
    else:
        return (time_string)
        
    (mins, secs) = time_string.split(splitter)
    return (mins + '.' + secs)

#def get_coach_data(filename):
#    try:
#        with open(filename) as f:
#            data = f.readline();
#        templ = data.strip().split(',')
#        return({'Name' : templ.pop(0),
#                'DOB'  : templ.pop(0),
#                'Times': str(sorted(set([sanitize(t) for t in templ]))[0:3])})
#    except IOError as ioerr:
#        print('File error: ' + str(ioerr))
#        return None

def get_coach_data2(filename):
    try:
        with open(filename) as f:
            data = f.readline();
        templ = data.strip().split(',')
        return (AthleteList(templ.pop(0), templ.pop(0), templ));
    except IOError as ioerr:
        print('File error: ' + str(ioerr))
        return None

#james = get_coach_data2('james2.txt')
#julie = get_coach_data2('julie2.txt')
#mikey = get_coach_data2('mikey2.txt')
#sarah = get_coach_data2('sarah2.txt')
#
#print(james.name + "'s fastest times are: " + str(james.top3()))
#print(julie.name + "'s fastest times are: " + str(julie.top3()))
#print(mikey.name + "'s fastest times are: " + str(mikey.top3()))
#print(sarah.name + "'s fastest times are: " + str(sarah.top3()))

