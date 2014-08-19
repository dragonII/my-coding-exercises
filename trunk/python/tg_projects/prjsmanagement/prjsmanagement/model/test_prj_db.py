from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Integer, Column, String, Unicode
from sqlalchemy import ForeignKey
from sqlalchemy.orm import relationship, backref

engine = create_engine('mysql://root:123456@127.0.0.1:3306/prjmng', echo=True)

Base = declarative_base()


class Project(Base):
    __tablename__ = 'projects'

    prj_id = Column(Integer, autoincrement = True, primary_key = True)
    prj_name = Column(Unicode(255))
    prj_owner_id = Column(Integer, ForeignKey('employees.e_id'))
    prj_owner = relationship("Employee", backref = "projects")
    start_date = Column(Date, nullable = False)
    estimate_end_date = Column(Date, nullable = False)
    end_date = Column(Date, nullable = False)
    customer = Column(Unicode(255))
    delivered = Column(Boolean)
    delivered_date = Column(Date)

class Employee(Base):
    __tablename__ = 'employees'

    e_id = Column(Integer, primary_key = True)
    e_name = Column(Unicode(32))
    e_join_date = Column(Date)
    e_email = Column(String(64))
    e_phone = Column(String(32))


class P1_Status(Base):
    __tablename__ = 'p1_status'

    p1_id = Column(Integer, primary_key = True)

    prj_id = Column(Integer, ForeignKey("projects.prj_id"))
    prj_name = relationship("Project", backref="p1_status")
    p1_owner_id = Column(Integer, ForeignKey("employees.e_id"))
    p1_end_date = Column(Date)

class P2_Status(DeclarativeBase):
    __tablename__ = 'p2_status'

    p2_id = Column(Integer, primary_key = True)

    prj_id =  Column(Integer, ForeignKey("projects.prj_id"))
    prj_name = relationship("Project", backref="p2_status")
    p2_owner = Column(Integer, ForeignKey("employees.e_id"))
    p2_end_date = Column(Date)

from sqlalchemy.orm import sessionmaker
Session = sessionmaker(bind = engine)

session = Session()
