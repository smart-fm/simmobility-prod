import sys
import csv
import json
import datetime
import argparse

import dcm

def write_outfile_head(f_out, all_choices, input_head, with_input):
    if with_input:
        for col in input_head:
            f_out.write('\"%s\",'%(col))
    
    f_out.write(','.join(['\"Util.%s\",\"Prob.%s\"'%(c,c) for c in all_choices]))
    f_out.write('\n')
    #f_out.write(',\"Sim.Choice\"\n')

def write_outfile_row(f_out, all_choices, utility, probability, final_choice, row, with_input):
    if with_input:
        for col in row: f_out.write('\"%s\",'%(col))
        
    f_out.write(','.join(['\"%f\",\"%f\"'%(utility[c],probability[c]) for c in all_choices]))
    f_out.write('\n')
    #f_out.write(',\"%s\"\n'%final_choice)
    
def do_main(file_model, file_csv, file_out, test_run=False, out_with_input=True):
    mnl = dcm.MultinomialLogit()
    mnl.load_model(file_model)
    all_choices = dcm.get_choicelist(mnl.Choiceset, True)
    print all_choices
    
    with open(file_csv,'rb') as f_in, open(file_out,'w') as f_out:
        reader = csv.reader(f_in, delimiter='\t')
        header = reader.next()
        header_idx = {}
        for i,c in enumerate(header):
            header_idx[c] = i
        print header
        
        write_outfile_head(f_out,all_choices,header,out_with_input)
        for row in reader: 
            data = {}
            for k,i in header_idx.items():
                data[k] = row[i]
            #print data
            
            (utility, probability, final_choice) = mnl.simulate(data)
            
            write_outfile_row(f_out, all_choices, utility, probability, final_choice, row, out_with_input)
            
            if test_run: break
            idx = reader.line_num
            sys.stdout.write('\r%d'%(idx))
            sys.stdout.flush()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("model", help="Model file in json format")
    parser.add_argument("input", help="input data file in csv format")
    parser.add_argument("output", help="output data file")
    parser.add_argument("--test", action="store_true",help="Test run, only process one iteration")
    args = parser.parse_args()
    
    Start = datetime.datetime.now()
    do_main(args.model,args.input,args.output, test_run=args.test)
    print '\rDone in %fs' %(datetime.datetime.now() - Start).total_seconds()
