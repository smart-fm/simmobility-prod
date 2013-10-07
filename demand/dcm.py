from decimal import Decimal
from math import *
from collections import OrderedDict
from copy import deepcopy
import json
import random
import datetime

model_file = None

def binary_search(a, x, lo=0, hi=None):
	if hi is None:
		hi = len(a)
	while lo != hi:
		mid = (lo+hi)//2
		midval = a[mid]
		if midval > x: 
			hi = mid
		elif midval < x:
			lo = mid+1
	return hi #or lo since hi == lo is true

def get_choicelist(choiceset, all_choices=False):
	lst = []
	if type(choiceset) is list: 
		lst = lst + choiceset
	else:
		for c in choiceset: 
			if all_choices: lst.append(c)		  
			lst = lst + get_choicelist(choiceset[c], all_choices)
	return lst

def assign_variables(parameters, variables):
	for p in parameters:
		exec '%s=%f'%(p,float(parameters[p])) in globals()
	for v in variables:
		exec '%s=%f'%(v,variables[v]) in globals()

def eval_function(func, parameters, variables):
	return Decimal(eval(func))

class MultinomialLogit:
	Choiceset = None
	Parameter = None
	Variable = None
	Utility = None
	Availability = None
	Scale = None
	
	__final_choices = None
	__all_choices = None
	
	def calculate_multinomial_logit_probability(self, choices, utility, availables, mu=1):
		probability = {}
		for c in choices:
			av = availables[c]
			### mu = scales[c]
			probability[c] = Decimal(av) * Decimal(mu*utility[c]).exp()
		evsum = sum(probability.values())
		#print 'probability:', probability
		#print 'evsum:', evsum
		for choice in probability:
			probability[choice] = probability[choice]/evsum
			##probability[choice] = choice+'*'
		return probability

	def calculate_nested_logit_probability(self, choiceset, utility, availables, scales):
		evmu = {}
		evsum = {}
		probability = {}
		for k in choiceset:
			mu = Decimal(scales[k])
			nest_evsum = 0
			for c in choiceset[k]:
				av = availables[c]
				evmuc = Decimal(av) * Decimal(mu*utility[c]).exp()
				evmu[c] = evmuc
				nest_evsum = nest_evsum + evmuc
			evsum[k] = nest_evsum
			
		sum_evsum_pow_muinv = 0
		for k in evsum:
			mu = Decimal(scales[k])
			sum_evsum_pow_muinv = sum_evsum_pow_muinv + pow(evsum[k], (1/mu))

		for k in choiceset:
			for c in choiceset[k]:
				mu = Decimal(scales[k])
				probability[c] = evmu[c] * Decimal(pow(evsum[k], (1/mu - 1))/sum_evsum_pow_muinv) if evsum[k] != 0 else 0

		return probability

	def calculate_probability(self, choiceset, utility, availables, scales, mu=1):
		probability = {}
		if type(choiceset) is list:
			##print 'list=',choiceset
			probability = self.calculate_multinomial_logit_probability(choiceset, utility, availables, mu)
		else:
			probability = self.calculate_nested_logit_probability(choiceset,utility,availables,scales)
	
		return probability
	
	def calculate_utility(self, result, choiceset, variables, scales, availables):
		utility = {}
		if type(choiceset) is list:
			for c in choiceset:
				utility[c] = eval_function(self.Utility[c],self.Parameter,variables) if availables[c] == 1 else 0
		else:
			for k in choiceset:
				#u1 = eval_function(self.Utility[k],self.Parameter,variables)
				u2s = self.calculate_utility(result, choiceset[k], variables, scales, availables)
				
				mu = scales[k]
				utility[k] = Decimal(sum([mu*Decimal(v).exp() for v in u2s.values()])).ln()/mu
				#utility[k] = u1 + Decimal(sum([mu*Decimal(v).exp() for v in u2s.values()])).ln()/mu
				###print k,'is calculated by',u2s.keys()
		result.update(utility)
		return utility
	
	def __pick_variable(self, data, default_value=0):
		variables = {}
		var_dict = self.Variable
		for k,v in var_dict.items():
			try:
				variables[k] = float(data[v]) if v in data else default_value
			except TypeError:
				print 'v', v, v in data
				print data
				raise TypeError()
			except ValueError:
				print 'v', v, v in data
				print data
				raise ValueError()
				
		return variables
	
	def __pick_choice_params(self, data, var_dict, default_value=1):
		params = {}
		choices = self.__all_choices
		for c in choices:
			if c in var_dict:
				v = var_dict[c]
				
				if v in data:
					params[c] = Decimal(data[v])
				else:
					try:
						params[c] = Decimal(v)
					except ValueError:
						params[c] = default_value
			else:
				params[c] = default_value
		return params
	
	def simulate(self, data, verbose=False):
		if verbose: print data
		variables = self.__pick_variable(data)
		if verbose: print 'variables',variables
		availables = self.__pick_choice_params(data, self.Availability)
		if verbose: print 'availables',availables
		scales = self.__pick_choice_params(data, self.Scale)
		if verbose: print 'scales',scales

		assign_variables(self.Parameter,variables)

		utility = {}
		self.calculate_utility(utility, self.Choiceset, variables, scales, availables)
		if verbose: print 'utility',utility
		
		probability = self.calculate_probability(self.Choiceset, utility, availables, scales)
		if verbose: print 'probability',probability

		final_choice = self.make_final_choice(probability)
		if verbose: print 'final_choice',final_choice
		
		return (utility,probability,availables,final_choice)
	
	def make_final_choice(self, probability):
		ps = []
		ps_prob = []
		cum_prob = 0
		for c in self.__final_choices:
			ps.append(c)
			cum_prob = cum_prob + probability[c]
			ps_prob.append(cum_prob)
		idx = binary_search(ps_prob, random.random())
		final_choice = ps[idx]
		return final_choice
	
	def load_model(self, file_json):
		with open(file_json,'r') as f:
			model = json.load(f, object_pairs_hook=OrderedDict)
			
			self.Choiceset = deepcopy(model['Choiceset'])
			self.__final_choices = get_choicelist(self.Choiceset)
			self.__all_choices = get_choicelist(self.Choiceset,True)
			
			self.Parameter = OrderedDict(model['Parameters'])
			self.Variable = OrderedDict(model['Variable'])
			self.Utility = OrderedDict(model['Utility'])
			self.Availability = OrderedDict(model['Availability'])
			self.Scale = OrderedDict(model['Scale'])
	
	#loader for dummy input files used for simulation
	def load_choiceset(self, model_name):
		file_json = model_file[model_name]
		with open(file_json,'r') as f:
			model = json.load(f, object_pairs_hook=OrderedDict)
			
			### compulsory
			self.Choiceset = deepcopy(model['Choiceset'])
			self.__final_choices = get_choicelist(self.Choiceset)
			self.__all_choices = get_choicelist(self.Choiceset,True)

			### optional #Availability to be made compulsory
			self.Availability = OrderedDict(model['Availability']) if 'Availability' in model else OrderedDict()
		
	def save_model(self, file_json):
		with open(file_json,'w') as f:
			model = OrderedDict()
			model['Choiceset'] = self.Choiceset
			model['Parameter'] = self.Parameter
			model['Variable'] = self.Variable
			model['Utility'] = self.Utility
			
			if len(self.Availability)>0: model['Availability'] = self.Availability
			if len(self.Scale)>0: model['Scale'] = self.Scale
			
			json.dump(model, f, indent=4, separators=(',', ': '))
	
	def __repr__(self):
		return '\n'.join(['%s']*12) % ('Choiceset:', self.Choiceset,
										'Parameter:', self.Parameter,
										'Variable:', self.Variable,
										'Utility:', self.Utility,
										'Availability:', self.Availability,
										'Scale:', self.Scale)
	
if __name__ == "__main__":
	
	Data_Head = {}
	Data_Content = []
	with open('swissmetro.dat','rb') as f:
		import csv
		reader = csv.reader(f, delimiter='\t')
		header = reader.next()
		for i,c in enumerate(header):
			Data_Head[c] = i
		for row in reader: 
			data = {}
			for k,i in Data_Head.items():
				data[k] = row[i]
			break
	#print data
	
	ml = MultinomialLogit()
	ml.load_model('01logit.json')
	print ml.simulate(data)
	
	ml.Choiceset.append('fly')
	ml.save_model('01logit-2.json')
	
