from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Integer, Column, String
from sqlalchemy import ForeignKey
from sqlalchemy.orm import relationship, backref

engine = create_engine('mysql://root:123456@127.0.0.1:3306/crud', echo = True)

Base = declarative_base()

class User(Base):
    __tablename__ = 'users'

    address = relationship("Address", order_by="Address.id", backref="user")
    id = Column(Integer, primary_key = True)
    name = Column(String(50))
    fullname = Column(String(50))
    password = Column(String(50))

    def __repr__(self):
        return "<User(name='%s', fullname='%s', password='%s')>" % (self.name, self.fullname, self.password)


class Address(Base):
    __tablename__ = 'addresses'

    id = Column(Integer, primary_key = True)
    email_address = Column(String(50), nullable = False)
    user_id = Column(Integer, ForeignKey('users.id'))
    user = relationship("User", backref=backref('addresses', order_by=id))

    def __repr__(self):
        return "<Address(email_address='%s')>" % self.email_address

#jack = User(name='Jack', fullname='Jack Bean', password='gjffdd')
#jack.addresses = [Address(email_address='jack@google.com'),
#                  Address(email_address='j25@yahoo.com')]
#
#print jack.addresses[1]
#print jack.addresses[1].user
#print "-----"
#print jack


from sqlalchemy.orm import sessionmaker
Session = sessionmaker(bind = engine)
session = Session()

#session.add(jack)
#session.commit()

jack = session.query(User).filter_by(name='jack').one()
print jack
print jack.addresses[1].user
