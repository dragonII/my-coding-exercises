from google.appengine.ext.webapp import template
html = template.render('templates/header.html', {'title': 'Report a Possible Sighting'})

html = html + template.render('templates/form_start.html', {})

# We need to generate the FORM fields n here... but now ?!

html = html + template.render('templates/form_end.html', {'sub_title': 'Submit Sighting'})
html = html + template.render('templates/footer.html', {'links': ''})

