#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

import random
from town import Town_layout

__all__ = ("Registrar_of_companies")

class Company_model:
    def __init__(self, min_head_count, max_head_count):
        assert(min_head_count <= max_head_count)
        self.min_head_count = min_head_count
        self.max_head_count = max_head_count

class Company:
    def __init__(self, name, max_head_count):
        self.name = name
        self.max_head_count = max_head_count
        self.staff = list()
        self.type = "Business"
        self.address = 0

    def set_type(self, type):
        self.type = type

    def set_address(self, address):
        self.address = address

    def is_hiring(self):
        return len(self.staff) < self.max_head_count

    def add_staff(self, person):
        if self.is_hiring():
            self.staff.append(person)

    def __repr__(self):
        return   "Company name='%s' type='%s' address=%d head_count=%d/%d" \
               % (self.name, self.type, self.address, len(self.staff), self.max_head_count)

class School(Company):
    def __init__(self, name, max_head_count):
        #super(School, self).__init__(name, max_head_count)
        Company.__init__(self, name, max_head_count)
        self.type = "School"
        self.students = list()

    def add_student(self, student):
        self.students.append(student)

    def __repr__(self):
        return   "School name='%s' type='%s' address=%d head_count=%d/%d student_count=%d" \
               % (self.name, self.type, self.address,
                  len(self.staff), self.max_head_count, len(self.students))

def create_school(name, small_company_model):
    head_count = random.randint(small_company_model.min_head_count,
                                small_company_model.max_head_count)
    school = School(name, head_count)
    return school

def create_companies(group, name_prefix, count, model):
    for i in range(count):
        head_count = random.randint(model.min_head_count, model.max_head_count)
        company = Company("%sCompany_%02d" % (name_prefix, i), head_count)
        group.append(company)

class Registrar_of_companies:
    small_company_model = Company_model(5, 20)
    medium_size_company_model = Company_model(20, 50)
    large_company_model = Company_model(40, 200)
    small_company_count = 100
    medium_size_company_count = 30
    large_company_count = 3

    small_companies = list()
    medium_size_companies = list()
    large_companies = list()

    schools = list()
    shops = list()
    restuarants = list()
    movie_houses = list()

    @staticmethod
    def genesis():
        klass = Registrar_of_companies

        school = create_school("School_0", klass.small_company_model)
        school.set_address(Town_layout.school_locations[0])
        klass.schools.append(school)
        klass.small_companies.append(school)
        school = create_school("School_1", klass.small_company_model)
        school.set_address(Town_layout.school_locations[1])
        klass.schools.append(school)
        klass.small_companies.append(school)

        create_companies(klass.small_companies, "Small",
                         klass.small_company_count - 2, klass.small_company_model)
        create_companies(klass.medium_size_companies, "MediumSize",
                         klass.medium_size_company_count, klass.medium_size_company_model)
        create_companies(klass.large_companies, "Large",
                         klass.large_company_count, klass.large_company_model)

        company = klass.small_companies[2]
        company.set_type("Movie House")
        company.set_address(Town_layout.movie_house_locations[0])
        klass.movie_houses.append(company)
        company = klass.small_companies[3]
        company.set_type("Movie House")
        company.set_address(Town_layout.movie_house_locations[1])
        klass.movie_houses.append(company)

        for company in klass.small_companies[4:20]:
            company.set_type("Shop")
            company.set_address(random.choice(Town_layout.shopping_center_locations))
            klass.shops.append(company)

        for company in klass.small_companies[20:32]:
            company.set_type("Restuarant")
            company.set_address(random.choice(Town_layout.restuarant_locations))
            klass.restuarants.append(company)

        for company in klass.small_companies[32:]:
            company.set_type("Business")
            company.set_address(random.choice(Town_layout.business_locations))

        for company in klass.medium_size_companies:
            company.set_type("Business")
            company.set_address(random.choice(Town_layout.business_locations))

        company = klass.large_companies[0]
        company.set_type("Departmental Store")
        company.set_address(48732)
        company = klass.large_companies[1]
        company.set_type("Hotel")
        company.set_address(65120)
        company = klass.large_companies[2]
        company.set_type("Bank")
        company.set_address(106946)

        random.shuffle(klass.small_companies)
        random.shuffle(klass.large_companies)

    @staticmethod
    def dump():
        klass = Registrar_of_companies
        for company in klass.small_companies:
            print company
        for company in klass.medium_size_companies:
            print company
        for company in klass.large_companies:
            print company

    @staticmethod
    def find_work(person):
        klass = Registrar_of_companies
        groups = (klass.small_companies, klass.medium_size_companies, klass.large_companies)
        while True:
            group = random.choice(groups)
            company = random.choice(group)
            if company.is_hiring:
                company.add_staff(person)
                return company
