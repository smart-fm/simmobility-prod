
#Returns the timestamp: -1 for invalid
def read_log_line(line, skipType='')
  return -1 if line.start_with? '#'
  return -1 unless m = line.match(/^[(]"([^"]+)" *, *([0-9]+) *, *[^{]+{[^}]+} *[)]$/)
  return -1 if m[1].downcase == skipType
  return m[2].to_i
end


def run_main()
  #First, read the "compare" file, saving all records based on time tick
  merge_in = {}
  File.open("compare.txt").each { |line|
    line.chomp!
    t = read_log_line(line)
    if t==-1
      puts "Skipping: #{line}"
      next
    end
    merge_in[t] = [] unless merge_in[t]
    merge_in[t].push(line)
  }

  #Next, open the real file, and write it to an output file, merging in as necessary.
  lastKnownTime = 0
  lineNumber = 0
  File.open('output_compare.txt', 'w') {|f|
    File.open("output.simmob_full.txt").each { |line|
      line.chomp!
      t = read_log_line(line, 'signal')
      lineNumber += 1

      if t==-1
        puts "Skipping: #{line}"
        next
      end

      #If we've advanced, merge
      if t > lastKnownTime
        (lastKnownTime...t).each{|tick|
          next unless merge_in.has_key? tick
          merge_in[tick].each{|m_line|
            f.write("#{m_line}\n")
          }
        }
      elsif t < lastKnownTime 
        raise "Error; time ticks out of order (#{lastKnownTime}...#{t})\n...on line #{lineNumber}"
      end

      #Finally, write the line
      f.write("#{line}\n")
      lastKnownTime = t
    }
  }


end




if __FILE__ == $PROGRAM_NAME
  run_main()
end




