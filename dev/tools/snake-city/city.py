#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import random, sys
from company import Registrar_of_companies
from person import Person

__all__ = ("City")

class Trip:
    def __init__(self, from_id, to_id, mode, start_time):
        self.from_id = from_id
        self.to_id = to_id
        self.mode = mode
        self.start_time = start_time

def generate_school_trips(student, trip_chains, activities):
    name = "student name='%s'" % student.name
    trip =   "from home-address='%s' to school-address='%s'" \
           % (student.address, student.school.address)
    start_time = "7:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
    if "car" == student.mode:
        print "Person name='%s' drives" % student.driver.name, name, trip, "at", start_time
    else:
        print name, "walks", trip, "at", start_time

    key = "home-%d" % student.address
    from_id = activities[key].id
    key = "school-%d" % student.school.address
    to_id = activities[key].id
    trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

    # catch a movie once in every 100 school days.
    if random.random() < 0.01:
        movie_house = random.choice(Registrar_of_companies.movie_houses)
        trip =   "walks from school-address='%s' to movie-house-address='%s'" \
               % (student.school.address, movie_house.address)
        start_time = "3:%02d:%02d" % (random.randint(0, 45), random.randint(0, 59))
        print name, trip, "at", start_time

        from_id = to_id
        key = "movie-%d" % movie_house.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

        trip =   "walks from movie-house-address='%s' to home-address='%s'" \
               % (movie_house.address, student.address)
        start_time = "6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, trip, "at", start_time

        from_id = to_id
        key = "home-%d" % student.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Walk", start_time))
    else:
        trip =   "walks from school-address='%s' to home-address='%s'" \
               % (student.school.address, student.address)
        start_time = "3:%02d:%02d" % (random.randint(0, 45), random.randint(0, 59))
        print name, trip, "at", start_time

        from_id = to_id
        key = "home-%d" % student.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

def generate_work_trips(person, trip_chains, activities):
    name = "Person name='%s'" % person.name
    if "car" == person.mode:
        #if len(person.passengers):
        #    print name, " drives from school-address='%s'"
        trip =   "drives from home-address='%s' to work-address='%s'" \
               % (person.address, person.company.address)
        start_time = "8:%02d:%02d" % (random.randint(0, 30), random.randint(0, 59))
        print name, trip, "at", start_time

        key = "home-%d" % person.address
        from_id = activities[key].id
        key = "work-%d" % person.company.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Car", start_time))
    else:
        trip =   "walks from home-address='%s' to work-address='%s'" \
               % (person.address, person.company.address)
        start_time = "8:%02d:%02d" % (random.randint(0, 30), random.randint(0, 59))
        print name, trip, "at", start_time

        key = "home-%d" % person.address
        from_id = activities[key].id
        key = "work-%d" % person.company.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

    # Time for lunch
    cafe = random.choice(Registrar_of_companies.restuarants)
    trip =   "walks from work-address='%s' to restuarant-address='%s' for lunch" \
           % (person.company.address, cafe.address)
    start_time = "12:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
    print name, trip, "at", start_time

    from_id = to_id
    key = "lunch-%d" % cafe.address
    to_id = activities[key].id
    trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

    # Back to the office
    trip =   "walks from restuarant-address='%s' to work-address='%s'" \
           % (cafe.address, person.company.address)
    start_time = "12:%02d:%02d" % (random.randint(45, 59), random.randint(0, 59))
    print name, trip, "at", start_time

    from_id = to_id
    key = "work-%d" % person.company.address
    to_id = activities[key].id
    trip_chains.append(Trip(from_id, to_id, "Walk", start_time))

    # 20 % chance of having dinner before going home
    if random.random() < 0.2:
        restuarant = random.choice(Registrar_of_companies.restuarants)
        trip =   "from office-address='%s' to resturant-address='%s'" \
               % (person.company.address, restuarant.address)
        mode = "drives" if "car" == person.mode else "walks"
        start_time = "6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, mode, trip, "for dinner at", start_time

        from_id = to_id
        key = "dinner-%d" % restuarant.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Car" if "car" == person.mode else "Walk", start_time))

        trip =   "from restuarant-address='%s' to home-address='%s'" \
               % (restuarant.address, person.address)
        start_time = "8:%02d:%02d" % (random.randint(0, 15), random.randint(0, 59))
        print name, mode, trip, "at", start_time

        from_id = to_id
        key = "home-%d" % person.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Car" if "car" == person.mode else "Walk", start_time))
    else:
        trip = "from office-address='%s' to home-address='%s'" \
               % (person.company.address, person.address)
        mode = "drives" if "car" == person.mode else "walks"
        start_time = "6:%02d:%02d" % (random.randint(30, 59), random.randint(0, 59))
        print name, mode, trip, "at", start_time

        from_id = to_id
        key = "home-%d" % person.address
        to_id = activities[key].id
        trip_chains.append(Trip(from_id, to_id, "Car" if "car" == person.mode else "Walk", start_time))

class Activity:
    def __init__(self, type, primary, flexible, location, id):
        self.type = type
        self.primary = primary
        self.flexible = flexible
        self.location = location
        self.id = id

    def __repr__(self):
        return   "Activity type=%s is%s-primary is%s-flexible location=%d" \
               % (self.type, "" if self.primary else "-not", "" if self.flexible else "-not", self.location)

class City:
    def __init__(self, population_size):
        self.residents = list()
        self.population_size = population_size
        self.activities = dict()
        self.trip_chains = list()

    def genesis(self):
        while len(self.residents) < self.population_size:
            Person(self)

    def dump(self):
        for person in self.residents:
            print person

    def generate_home_activities(self):
        for person in self.residents:
            key = "home-%d" % person.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("Home", True, False, person.address, id)

    def generate_work_place_activities(self, companies):
        for company in companies:
            if len(company.staff):
                key = "work-%d" % company.address
                if key not in self.activities:
                    id = len(self.activities)
                    self.activities[key] = Activity("Work", True, False, company.address, id)

    def generate_shopping_activities(self):
        for shop in Registrar_of_companies.shops:
            key = "shopping-%d" % shop.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("Shopping", False, True, shop.address, id)

    def generate_school_activities(self):
        for school in Registrar_of_companies.schools:
            key = "school-%d" % school.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("School", True, False, school.address, id)

    def generate_lunch_activities(self):
        for restuarant in Registrar_of_companies.restuarants:
            key = "lunch-%d" % restuarant.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("Lunch", False, True, restuarant.address, id)

    def generate_dinner_activities(self):
        for restuarant in Registrar_of_companies.restuarants:
            key = "dinner-%d" % restuarant.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("Dinner", False, True, restuarant.address, id)

    def generate_movie_activities(self):
        for movie_house in Registrar_of_companies.movie_houses:
            key = "movie-%d" % movie_house.address
            if key not in self.activities:
                id = len(self.activities)
                self.activities[key] = Activity("Movie", False, True, movie_house.address, id)

    def generate_activities(self):
        self.generate_home_activities()
        self.generate_work_place_activities(Registrar_of_companies.small_companies)
        self.generate_work_place_activities(Registrar_of_companies.medium_size_companies)
        self.generate_work_place_activities(Registrar_of_companies.large_companies)
        self.generate_shopping_activities()
        self.generate_school_activities()
        self.generate_lunch_activities()
        self.generate_dinner_activities()
        self.generate_movie_activities()

    def generate_trips(self):
        self.generate_activities()
        #for key, activity in self.activities.iteritems():
        #    print key, activity
        for person in self.residents:
            if person.father:
                child = person
                # a child is either in school or is at home; a toddler never gets to go anywhere.
                if child.is_student:
                    generate_school_trips(child, self.trip_chains, self.activities)
            elif person.company:
                generate_work_trips(person, self.trip_chains, self.activities)

    def dump_sql_statements(self):
        self.dump_activity_spec_insert_statements()
        print >>sys.stderr
        self.dump_trip_chains_insert_statements()

    def dump_activity_spec_insert_statements(self):
        activity_types = {"Home" : 1, "Work" : 2, "Shopping" : 3,
                          "School" : 4, "Lunch" : 5, "Dinner" : 6, "Movie" : 8 }
        sql = 'INSERT INTO "Activity_Spec"("Activity_Id", "Activity_Type", "Primary_Activity", '
        sql += '"Flexible_Activity", "Location_Id")'
        activities = self.activities.values()
        activities.sort(key = lambda activity : activity.id) 
        for activity in activities:
            print >>sys.stderr, sql, 'VALUES(%d, %d, %s, %s, %d);' % (100 + activity.id,
                                                           activity_types[activity.type],
                                                           "TRUE" if activity.primary else "FALSE",
                                                           "TRUE" if activity.flexible else "FALSE",
                                                           activity.location)

    def dump_trip_chains_insert_statements(self):
        mode_choices = {"Car" : 1, "Walk" : 2, "Bus" : 3, "Taxi" : 4,
                        "MRT" : 5, "Car Sharing" : 6, "Cycle" : 7, "Bike" : 8}
        sql = 'INSERT INTO "Trip_Chains"("From_Activity_Id", "To_Activity_Id", '
        sql += '"Mode_Of_Travel_Id", "Start_Time")'
        for trip in self.trip_chains:
            print >>sys.stderr, sql, 'VALUES(%d, %d, %d, "%s");' % (trip.from_id, trip.to_id,
                                                                    mode_choices[trip.mode],
                                                                    trip.start_time)
