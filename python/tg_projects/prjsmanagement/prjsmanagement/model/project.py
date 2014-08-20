from sqlalchemy import Table, ForeignKey, Column, Text
from sqlalchemy.types import Unicode, Integer, Date, String, Boolean
from sqlalchemy.orm import relationship, backref

from prjsmanagement.model import DeclarativeBase

employee_projects = Table('employee_projects', DeclarativeBase.metadata,
                       Column('employee_id', Integer, ForeignKey('employees.e_id')),
                       Column('project_id', Integer, ForeignKey('projects.prj_id'))
                    )
#class Employee_Project(DeclarativeBase):
#    __tablename__ = 'employee_projects'
#
#    id = Column(Integer, primary_key = True)
#    employee_id = Column(Integer, ForeignKey('employees.e_id'))
#    project_id = Column(Integer, ForeignKey('projects.prj_id'))

class Project(DeclarativeBase):
    __tablename__ = 'projects'

    prj_id = Column(Integer, autoincrement = True, primary_key = True)
    prj_name = Column(Unicode(255))
    prj_owner_id = Column(Integer, ForeignKey('employees.e_id'))
    #prj_owner = relation("Employee", backref = "projects")
    prj_owner = relationship("Employee", backref = backref("projects", order_by=prj_id))
    start_date = Column(Date, nullable = False)
    estimate_end_date = Column(Date, nullable = False)
    end_date = Column(Date)
    customer = Column(Unicode(255))
    delivered = Column(Boolean, nullable = False)
    delivered_date = Column(Date)

class Employee(DeclarativeBase):
    __tablename__ = 'employees'

    e_id = Column(Integer, primary_key = True)
    e_name = Column(Unicode(32))
    e_join_date = Column(Date)
    e_email = Column(String(64))
    e_phone = Column(String(32))
    e_projects = relationship('Project', secondary=employee_projects, backref='owners')


class P1_Status(DeclarativeBase):
    __tablename__ = 'p1_status'

    p1_id = Column(Integer, primary_key = True)

    prj_id = Column(Integer, ForeignKey("projects.prj_id"))
    prj_name = relationship("Project", backref="p1_status")
    p1_owner_id = Column(Integer, ForeignKey("employees.e_id"))
    p1_end_date = Column(Date)
    p1_finished = Column(Boolean, nullable = False)
    p1_comment = Column(Text)


class P2_Status(DeclarativeBase):
    __tablename__ = 'p2_status'

    p2_id = Column(Integer, primary_key = True)

    prj_id =  Column(Integer, ForeignKey("projects.prj_id"))
    prj_name = relationship("Project", backref="p2_status")
    p2_owner_id = Column(Integer, ForeignKey("employees.e_id"))
    p2_end_date = Column(Date)
    p2_finished = Column(Boolean, nullable = False)
    p2_comment = Column(Text)
