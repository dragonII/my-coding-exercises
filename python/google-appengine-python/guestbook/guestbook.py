import os
import urllib

from google.appengine.api import users
from google.appengine.ext import ndb

import webapp2
import jinja2

JINJA_ENVIRONMENT = jinja2.Environment(
        loader = jinja2.FileSystemLoader(os.path.dirname(__file__)),
        extensions = ['jinja2.ext.autoescape'],
        autoescape = True)


MAIN_PAGE_FOOTER_TEMPLATE = """\
    <form action="/sign?%s" method="post">
        <div><textarea name="content" rows="3" cols="60"></textarea></div>
        <div><input type="submit" value="Sign Guestbook"></div>
    </form>
    <hr>
    <form>Guestbook name:
        <input value="%s" name="guestbook_name">
        <input type="submit" value="switch">
    </form>
    <a href="%s">%s</a>
</body>
</html>
"""

DEFAULT_GUESTBOOK_NAME = 'default_guestbook'

# We set a parent key on the 'Greetings' to ensure that they are all in the same
# entity group. Queries across the single entity group will be consistent.
# However, the write rate should be limited to ~1/second.

def guestbook_key(guestbook_name = DEFAULT_GUESTBOOK_NAME):
    """Construct a Datastore key for a Guestbook entity with guestbook_name."""
    return ndb.Key('Guestbook', guestbook_name)

class Greeting(ndb.Model):
    """Models an individual Guestbook entry."""
    author = ndb.UserProperty()
    content = ndb.StringProperty(indexed = False)
    date = ndb.DateTimeProperty(auto_now_add = True)

class MainPage(webapp2.RequestHandler):
    def get(self):
        #self.response.write('<html><body>')
        guestbook_name = self.request.get('guestbook_name', DEFAULT_GUESTBOOK_NAME)

        # Ancestor Queries, as shown here, are strongly consistent with the High
        # Replication Datastore. Queries that span entity groups are evenually
        # consistent. If we omitted the ancestor from this query there would be
        # a slight chance that Greeting that had just been written would not
        # show up in a query.
        greetings_query = Greeting.query(ancestor=guestbook_key(guestbook_name)).order(-Greeting.date)
        greetings = greetings_query.fetch(10)

        #for greeting in greetings:
        #    if greeting.author:
        #        self.response.write('<b>%s</b> wrote' % greeting.author.nickname())
        #    else:
        #        self.response.write('An anonymous person wrote:')
        #    self.response.write('<blockquote>%s</blockquote>' % cgi.escape(greeting.content))

        if users.get_current_user():
            url = users.create_logout_url(self.request.uri)
            url_link_text = 'Logout'
        else:
            url = users.create_login_url(self.request.uri)
            url_link_text = 'Login'

        template_values = {
            'greetings': greetings,
            'guestbook_name': urllib.quote_plus(guestbook_name),
            'url': url,
            'url_link_text': url_link_text,
            }

        template = JINJA_ENVIRONMENT.get_template('index.html')
        self.response.write(template.render(template_values))

        ## Write the submission form and the footer of the page
        #sign_query_params = urllib.urlencode({'guestbook_name': guestbook_name})
        #self.response.write(MAIN_PAGE_FOOTER_TEMPLATE %
        #                        (sign_query_params, cgi.escape(guestbook_name),
        #                        url, url_link_text))


class Guestbook(webapp2.RequestHandler):

    def post(self):
        # We set the same parent key on the 'Greeting' to ensure each Greeting
        # is in the same entity group. Queries across the single entity group
        # will be consistent. However, the write rate to a single entity group
        # should be limited to ~1/second.
        guestbook_name = self.request.get('guestbook_name', DEFAULT_GUESTBOOK_NAME)
        greeting = Greeting(parent = guestbook_key(guestbook_name))

        if users.get_current_user():
            greeting.author = users.get_current_user()

        greeting.content = self.request.get('content')
        greeting.put()

        query_params = {'guestbook_name': guestbook_name}
        self.redirect('/?' + urllib.urlencode(query_params))

application = webapp2.WSGIApplication([('/', MainPage), ('/sign', Guestbook), ], debug = True)
