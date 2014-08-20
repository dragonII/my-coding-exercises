# -*- coding: utf-8 -*-
"""Main Controller"""

from tg import expose, flash, require, url, lurl, request, redirect, tmpl_context
from tg.i18n import ugettext as _, lazy_ugettext as l_
from tg.exceptions import HTTPFound
from tg import predicates
from prjsmanagement import model
from prjsmanagement.controllers.secure import SecureController
from prjsmanagement.model import DBSession, metadata
from tgext.admin.tgadminconfig import BootstrapTGAdminConfig as TGAdminConfig
from tgext.admin.controller import AdminController

from prjsmanagement.lib.base import BaseController
from prjsmanagement.controllers.error import ErrorController

from tg.i18n import ugettext as _
from tg.i18n import lazy_ugettext as l_

from prjsmanagement.model import Project, Employee, P1_Status, P2_Status

__all__ = ['RootController']

from sqlalchemy import asc, desc
from tw2.forms.datagrid import Column
import genshi

class SortableColumn(Column):
    def __init__(self, title, name):
        super(SortableColumn, self).__init__(name)
        self._title_ = title
    
    def set_title(self, title):
        self._title_ = title

    def get_title(self):
        current_ordering = request.GET.get('ordercol')
        if current_ordering and current_ordering[1:] == self.name:
            current_ordering = '-' if current_ordering[0] == '+' else '+'
        else:
            current_ordering = '+'
        current_ordering += self.name

        new_params = dict(request.GET)
        new_params['ordercol'] = current_ordering

        new_url = url(request.path_url, params = new_params)
        return genshi.Markup('<a href="%(page_url)s">%(title)s</a>' % dict(page_url=new_url, title=self._title_))

    title = property(get_title, set_title)

from tw2.forms import DataGrid

project_status_grid = DataGrid(
        fields = [
                 SortableColumn(_('ID'), 'prj_id'),
                 SortableColumn(_('Owner'), 'prj_owner.e_name'),
                 SortableColumn(_('Prj Name'), 'prj_name'),
                 SortableColumn(_('Start Date'), 'start_date'),
                 SortableColumn(_('Est End Date'), 'estimate_end_date'),
                 SortableColumn(_('Delivered'), 'delivered')
                 #(_('P1 Status'), 'p1_end_date')
                 ])

class RootController(BaseController):
    secc = SecureController()
    admin = AdminController(model, DBSession, config_type=TGAdminConfig)

    error = ErrorController()

    def _before(self, *args, **kw):
        tmpl_context.project_name = "prjsmanagement"

    def order_by_owner(self, ordering):
        if 'e_name' in ordering:
            data = DBSession.query(Project).join(Employee)

    @expose('prjsmanagement.templates.index')
    def index(self, *args, **kw):
        data = DBSession.query(Project)
        ordering = kw.get('ordercol')
        if ordering:
            order_key = ordering[1:]
            if 'e_name' in ordering:
                data = data.join(Employee)
                order_key = order_key.split('.')[1]
            if ordering[0] == '+':
                #data = data.order_by(asc(ordering[1:]))
                data = data.order_by(asc(order_key))
            elif ordering[0] == '-':
                #data = data.order_by(desc(ordering[1:]))
                data = data.order_by(desc(order_key))
        return dict(page='index', grid = project_status_grid, data = data)

    @expose('prjsmanagement.templates.about')
    def about(self):
        """Handle the 'about' page."""
        return dict(page='about')

    @expose('prjsmanagement.templates.environ')
    def environ(self):
        """This method showcases TG's access to the wsgi environment."""
        return dict(page='environ', environment=request.environ)

    @expose('prjsmanagement.templates.data')
    @expose('json')
    def data(self, **kw):
        """This method showcases how you can use the same controller for a data page and a display page"""
        return dict(page='data', params=kw)
    @expose('prjsmanagement.templates.index')
    @require(predicates.has_permission('manage', msg=l_('Only for managers')))
    def manage_permission_only(self, **kw):
        """Illustrate how a page for managers only works."""
        return dict(page='managers stuff')

    @expose('prjsmanagement.templates.index')
    @require(predicates.is_user('editor', msg=l_('Only for the editor')))
    def editor_user_only(self, **kw):
        """Illustrate how a page exclusive for the editor works."""
        return dict(page='editor stuff')

    @expose('prjsmanagement.templates.login')
    def login(self, came_from=lurl('/')):
        """Start the user login."""
        login_counter = request.environ.get('repoze.who.logins', 0)
        if login_counter > 0:
            flash(_('Wrong credentials'), 'warning')
        return dict(page='login', login_counter=str(login_counter),
                    came_from=came_from)

    @expose()
    def post_login(self, came_from=lurl('/')):
        """
        Redirect the user to the initially requested page on successful
        authentication or redirect her back to the login page if login failed.

        """
        if not request.identity:
            login_counter = request.environ.get('repoze.who.logins', 0) + 1
            redirect('/login',
                params=dict(came_from=came_from, __logins=login_counter))
        userid = request.identity['repoze.who.userid']
        flash(_('Welcome back, %s!') % userid)

        # Do not use tg.redirect with tg.url as it will add the mountpoint
        # of the application twice.
        return HTTPFound(location=came_from)

    @expose()
    def post_logout(self, came_from=lurl('/')):
        """
        Redirect the user to the initially requested page on logout and say
        goodbye as well.

        """
        flash(_('We hope to see you soon!'))
        return HTTPFound(location=came_from)
