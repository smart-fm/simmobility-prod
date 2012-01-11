#!/usr/bin/env python

# Copyright Singapore-MIT Alliance for Research and Technology

class Error(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return value
