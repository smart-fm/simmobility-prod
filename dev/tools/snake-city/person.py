#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import random, sys
from town import Town_layout
from company import Registrar_of_companies

def generate_school_trips(student):
    name = "student name='%s'" % student.name
    trip =   "from home-address='%s' to school-address='%s'" \
           % (student.address, student.school.address)
    start_time = "at 7:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
    if "car" == student.mode:
        print "Person name='%s' drives" % student.driver.name, name, trip, start_time
    else:
        print name, "walks", trip, start_time

    # catch a movie once in every 100 school days.
    if random.random() < 0.01:
        movie_house = random.choice(Registrar_of_companies.movie_houses)
        trip =   "walks from school-address='%s' to movie-house-address='%s'" \
               % (student.school.address, movie_house.address)
        start_time = "at 3:%02d:%02d" % (random.randint(0, 45), random.randint(0, 59))
        print name, trip, start_time

        trip =   "walks from movie-house-address='%s' to home-address='%s'" \
               % (movie_house.address, student.address)
        start_time = "at 6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, trip, start_time
    else:
        trip =   "walks from school-address='%s' to home-address='%s'" \
               % (student.school.address, student.address)
        start_time = "at 3:%02d:%02d" % (random.randint(0, 45), random.randint(0, 59))
        print name, trip, start_time

def generate_work_trips(person):
    name = "Person name='%s'" % person.name
    if "car" == person.mode:
        #if len(person.passengers):
        #    print name, " drives from school-address='%s'"
        trip =   "drives from home-address='%s' to work-address='%s'" \
               % (person.address, person.company.address)
        start_time = "at 8:%02d:%02d" % (random.randint(0, 30), random.randint(0, 59))
        print name, trip, start_time
    else:
        trip =   "walks from home-address='%s' to work-address='%s'" \
               % (person.address, person.company.address)
        start_time = "at 8:%02d:%02d" % (random.randint(0, 30), random.randint(0, 59))
        print name, trip, start_time

    # Time for lunch
    cafe = random.choice(Registrar_of_companies.restuarants)
    trip =   "walks from work-address='%s' to restuarant-address='%s' for lunch" \
           % (person.company.address, cafe.address)
    start_time = "at 12:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
    print name, trip, start_time

    # Back to the office
    trip =   "walks from restuarant-address='%s' to work-address='%s'" \
           % (cafe.address, person.company.address)
    start_time = "at 12:%02d:%02d" % (random.randint(45, 59), random.randint(0, 59))
    print name, trip, start_time

    # 20 % chance of having dinner before going home
    if random.random() < 0.2:
        restuarant = random.choice(Registrar_of_companies.restuarants)
        trip =   "from office-address='%s' to resturant-address='%s'" \
               % (person.company.address, restuarant.address)
        mode = "drives" if "car" == person.mode else "walks"
        start_time = "at 6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, mode, trip, start_time

        trip =   "from restuarant-address='%s' to home-address='%s'" \
               % (restuarant.address, person.address)
        start_time = "at 8:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
        print name, mode, trip, start_time
    else:
        trip = "from office-address='%s' to home-address='%s'" \
               % (person.company.address, person.address)
        mode = "drives" if "car" == person.mode else "walks"
        start_time = "at 6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, mode, trip, start_time

class Activity:
    def __init__(self, type, primary, flexible, location):
        self.type = type
        self.primary = primary
        self.flexible = flexible
        self.location = location

    def __repr__(self):
        return   "Activity type=%s is%s-primary is%s-flexible location=%d" \
               % (self.type, "" if self.primary else "-not", "" if self.flexible else "-not", self.location)

class City:
    residents = list()
    max_population_size = 2000
    activities = dict()

    @staticmethod
    def genesis():
        while len(City.residents) < City.max_population_size:
            Person()

    @staticmethod
    def dump():
        for person in City.residents:
            print person

    @staticmethod
    def generate_home_activities():
        for person in City.residents:
            key = "home-%d" % person.address
            if key not in City.activities:
                City.activities[key] = Activity("Home", True, False, person.address)

    @staticmethod
    def generate_work_place_activities(companies):
        for company in companies:
            if len(company.staff):
                key = "work-%d" % company.address
                if key not in City.activities:
                    City.activities[key] = Activity("Work", True, False, company.address)

    @staticmethod
    def generate_shopping_activities():
        for shop in Registrar_of_companies.shops:
            key = "shopping-%d" % shop.address
            if key not in City.activities:
                City.activities[key] = Activity("Shopping", False, True, shop.address)

    @staticmethod
    def generate_school_activities():
        for school in Registrar_of_companies.schools:
            key = "school-%d" % school.address
            if key not in City.activities:
                City.activities[key] = Activity("School", True, False, school.address)

    @staticmethod
    def generate_lunch_activities():
        for restuarant in Registrar_of_companies.restuarants:
            key = "lunch-%d" % restuarant.address
            if key not in City.activities:
                City.activities[key] = Activity("Lunch", False, True, restuarant.address)

    @staticmethod
    def generate_dinner_activities():
        for restuarant in Registrar_of_companies.restuarants:
            key = "dinner-%d" % restuarant.address
            if key not in City.activities:
                City.activities[key] = Activity("Dinner", False, True, restuarant.address)

    @staticmethod
    def generate_movie_activities():
        for movie_house in Registrar_of_companies.movie_houses:
            key = "movie-%d" % movie_house.address
            if key not in City.activities:
                City.activities[key] = Activity("Movie", False, True, movie_house.address)

    @staticmethod
    def generate_activities():
        City.generate_home_activities()
        City.generate_work_place_activities(Registrar_of_companies.small_companies)
        City.generate_work_place_activities(Registrar_of_companies.medium_size_companies)
        City.generate_work_place_activities(Registrar_of_companies.large_companies)
        City.generate_shopping_activities()
        City.generate_school_activities()
        City.generate_lunch_activities()
        City.generate_dinner_activities()
        City.generate_movie_activities()

    @staticmethod
    def generate_trips():
        City.generate_activities()
        #for key, activity in City.activities.iteritems():
        #    print key, activity
        for person in City.residents:
            if person.father:
                child = person
                # a child is either in school or is at home; a toddler never gets to go anywhere.
                if child.is_student:
                    generate_school_trips(child)
            elif person.company:
                generate_work_trips(person)

    @staticmethod
    def dump_sql_statements():
        activity_types = {"Home" : 1, "Work" : 2, "Shopping" : 3,
                          "School" : 4, "Lunch" : 5, "Dinner" : 6, "Movie" : 8 }
        sql = 'INSERT INTO "Activity_Spec"("Activity_Type", "Primary_Activity", '
        sql += '"Flexible_Activity", "Location_Id")'
        for key, activity in City.activities.iteritems():
            print >>sys.stderr, sql, 'VALUES(%d, %s, %s, %d);' % (activity_types[activity.type],
                                                           "TRUE" if activity.primary else "FALSE",
                                                           "TRUE" if activity.flexible else "FALSE",
                                                           activity.location)

class Person:
    def __init__(self, attrs=None):
        self.name = "Person_%04d" % len(City.residents)
        City.residents.append(self)
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
                self.spouse = Person(attrs)

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
                    child = Person(attrs)
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
