#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import random
from town import Town_layout
from company import Registrar_of_companies

class Person:
    def __init__(self, city, attrs=None):
        self.name = "Person_%04d" % len(city.residents)
        city.residents.append(self)
        self.children = list()
        self.father = None
        self.mother = None
        self.spouse = None
        self.company = None

        if not attrs:
            # slightly more male than female in this city
            self.is_male = True if random.random() < 0.50005 else False
            # 60 % of population are married
            self.is_married = True if random.random() < 0.6 else False
            self.get_job()
            self.address = random.choice(Town_layout.residential_locations)

            # 80 % of population own a car
            self.mode = "car" if random.random() < 0.8 else "walk"
            self.passengers = list()

            if self.is_married:
                attrs = dict()
                attrs["sex"] = not self.is_male
                attrs["is_married"] = True
                attrs["spouse"] = self
                attrs["address"] = self.address
                self.spouse = Person(city, attrs)

                child_count = random.randint(0, 3)
                if child_count:
                    attrs = dict()
                    if self.is_male:
                        attrs["father"] = self
                        attrs["mother"] = self.spouse
                    else:
                        attrs["mother"] = self
                        attrs["father"] = self.spouse
                    attrs["address"] = self.address
                    child = Person(city, attrs)
                    self.add_child(child)
                    self.spouse.add_child(child)

        elif "spouse" in attrs:
            self.is_male = attrs["sex"]
            self.is_married = attrs["is_married"]
            self.spouse = attrs["spouse"]
            self.address = attrs["address"]
            # 90 % of spouses are also working.
            if random.random() < 0.9:
                self.get_job()

            # 80 % of population own a car
            self.mode = "car" if random.random() < 0.8 else "walk"
            self.passengers = list()

        else:
            self.father = attrs["father"]
            self.mother = attrs["mother"]
            self.address = attrs["address"]
            # child's age is between 1 to 18.  If child is 7 years old or younger,
            # then child stays at home.
            self.is_student = True if random.randint(1, 18) > 7 else False
            if self.is_student:
                self.school = random.choice(Registrar_of_companies.schools)
                self.school.add_student(self)
#                if "car" == self.father.mode or "car" == self.mother.mode:
#                    # 80 % chance of getting parent to drive child to school if both have cars;
#                    # 40 % chance if only one parent has a car
#                    if "car" == self.father.mode and "car" == self.mother.mode:
#                        self.mode = "car" if random.random() < 0.8 else "walk"
#                        self.driver = self.father if random.random() < 0.5 else self.mother
#                    else:
#                        self.mode = "car" if random.random() < 0.4 else "walk"
#                        self.driver = self.father if "car" == self.father.mode else self.mother
#                    self.driver.passengers.append(self)
#                else:
#                    self.mode = "walk"
                self.mode = "walk"

    def add_child(self, child):
        self.children.append(child)

    def get_job(self):
        prob = 0.9 if self.is_male else 0.8
        if random.random() < prob:
            self.company = Registrar_of_companies.find_work(self)

    def __repr__(self):
        if self.father:
            bio_data =   "Child name='%s' father='%s' mother='%s'" \
                       % (self.name, self.father.name, self.mother.name)
            if self.is_student:
                bio_data += " school='%s' mode='%s'" % (self.school.name, self.mode)
            else:
                bio_data += " is-toddler"
            return bio_data

        elif not self.is_married:
            bio_data =   "Person name='%s' %s single mode='%s'" \
                       % (self.name, "male" if self.is_male else "female", self.mode)
            if self.company:
                bio_data += " company='%s'" % self.company.name
            else:
                bio_data += " stay-at-home"
            return bio_data

        else:
            bio_data =   "Person name='%s' %s married spouse='%s'" \
                       % (self.name, "male" if self.is_male else "female", self.spouse.name)
            if self.company:
                bio_data += " company='%s'" % self.company.name
            else:
                bio_data += " stay-at-home"
            for child in self.children:
                bio_data += " child='%s'" % child.name
            bio_data += " mode='%s'" % self.mode
            return bio_data
