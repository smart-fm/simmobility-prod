clean_hh_choiceset <- function(df.hh_choiceset,df.household){
  df.tmp <- df.hh_choiceset %>%
    filter(day != 'day') %>%
    mutate(day = as.integer(day),
           householdId = as.integer(householdId),
           unitId1 = as.integer(unitId1))
  
  # unit_ids <- df.household$fm_unit_id
  # names(unit_ids) <- df.household$hh_id
  # unit_ids['3142']
  # 
  # str(df.tmp)
}

summarize_hh_choiceset <- function(df.hh_choiceset_clean){
  
  df.results <- data.frame()
  days = unique(df.hh_choiceset_clean$day)
  
  for(i in days){
    #i=days[1]
    df.day_tmp <- df.hh_choiceset_clean %>%
      filter(day==i) %>%
      select(-day,-householdId)
    
    tmp<-as.data.frame(table(as.vector(as.matrix(df.day_tmp))))
    colnames(tmp) <-c('unit_id','num')
    tmp$day = i
    df.results <- rbind(df.results,tmp)
  }
  
  df.results
}

compare_units <- function(df.console_output, df.daily_units, 
                           df.hh_choiceset_clean, df.bids, 
                           pth.figure){
  #unit perspective
  df.tmp_units <- df.console_output %>%
    #mutate(day=day-1) %>%
    filter(day>=0) %>%
    select(day,num_hunits)
  
  df.tmp_units <- df.daily_units %>%
    group_by(day) %>%
    summarise(num_daily = length(unique(unitId)))%>%
    merge(df.tmp_units,by='day',all=T)
  
  
  
  # df.tmp_units <- df.hh_choiceset_clean %>%
  #   group_by(day) %>%
  #   summarise(num_choiceset = ) %>%
  #   merge(df.tmp_units,by='day',all=T)
  
  
  num_choiceset = c()
  days <- unique(df.hh_choiceset_clean$day)
  for(i in days){
    #i = days[1]
    df.one_day <- df.hh_choiceset_clean %>%
      filter(day == i) %>%
      select(-day,-householdId)
    unit_ids <- unique(as.vector(as.matrix(df.one_day)))
    unit_ids <- unit_ids[!is.na(unit_ids)]
    num_choiceset <- c(num_choiceset,length(unit_ids))
  }
  df.tmp <- data.frame(days,num_choiceset)
  df.tmp_units <- df.tmp %>%
    rename(day=days) %>%
    merge(df.tmp_units,by='day',all=T)
  
  
  df.tmp_units <- df.bids %>%
    group_by(bid_timestamp) %>%
    summarise(num_bid_units = length(unique(unit_id))) %>%
    rename(day = bid_timestamp) %>%
    merge(df.tmp_units,by='day',all=T)
  
  
  ggplot(df.tmp_units) +
    geom_line(aes(x = day, y = num_hunits,colour="Console")) +
    geom_line(aes(x = day, y = num_daily,colour="dailyHousingMarketUnits.csv")) +
    geom_line(aes(x = day, y = num_choiceset,colour="HHChoiceSet.csv")) +
    geom_line(aes(x = day, y = num_bid_units,colour="Bids.csv")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Number of Units")
   # labels = c("Console", "dailyHousingMarketUnits.csv",
   #                        "HHChoiceSet.csv"))
  
  doc.plt = paste0(pth.figure,'check_','line_units_diff.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  df.tmp_units
  
}


compare_bidders <- function(df.console_output,df.hh_choiceset_clean,
                            df.bids, pth.figure){
  #bidder perspective
  df.tmp_bidder <- df.console_output %>%
    mutate(day=day-1) %>%
    filter(day>=0) %>%
    rename(num_bidders_console = num_bidders,
           num_bids_console = num_bids,
           num_accept_console = num_accept)%>%
    select(day,num_bidders_console,num_bids_console,num_accept_console)
  
  df.tmp_bidder <- df.hh_choiceset_clean %>%
    group_by(day) %>%
    summarise(num_bidder_choiceset_all = n(),
              num_bidder_choiceset_bid = sum(!is.na(unitId1))) %>%
    merge(df.tmp_bidder,by=1,all=T)
  
  df.tmp_bidder <- df.bids %>%
    group_by(bid_timestamp) %>%
    summarise(num_bids=n(),num_bids_win=sum(bid_status)) %>%
    rename(day = bid_timestamp) %>%
    merge(df.tmp_bidder,by=1,all=T)
  
  
  
  # df.tmp <- df.tmp_bidder %>%
  #   filter(#num_bidder_choiceset>num_bidders_console|
  #     num_bidders_console<num_bidder_choiceset_bid)
  
  
  
  ggplot(df.tmp_bidder) +
    geom_line(aes(x = day, y = num_bidders_console ,colour="Console")) +
    geom_line(aes(x = day, y = num_bids, colour="bids.csv")) +
    geom_line(aes(x = day, y = num_bidder_choiceset_bid,colour="HHChoiceSet.csv")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Bidder")

  doc.plt = paste0(pth.figure,'check_','line_bidders_count.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  
  ggplot(df.tmp_bidder) +
    geom_line(aes(x = day, y = num_bids_console - num_bids,colour="All")) +
    geom_line(aes(x = day, y = num_accept_console -num_bids_win,
                  colour="Successful Bids")) +
    #geom_line(aes(x = day, y = bids_cum,colour="lightskyblue")) +
    xlab('Day') + ylab('Difference')  +
    scale_color_discrete(name = "Bids Difference")
  #  annotate("text", x = 100, y = -2000, label = "Some text")
  #,labels = c("Bids (Console-CSV)", "Bidder (Console-Choiceset)"))

  doc.plt = paste0(pth.figure,'check_','line_bids_diff.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)


  ggplot(df.tmp_bidder) +
    geom_line(aes(x = day, y = num_bidders_console - num_bidder_choiceset_bid,
                  colour="Console - ChoiceSet")) +
    #geom_line(aes(x = day, y = bids_cum,colour="lightskyblue")) +
    xlab('Day') + ylab('Difference')  +
    scale_color_discrete(name = "Bidder Difference")
  #  annotate("text", x = 100, y = -2000, label = "Some text")
  #,labels = c("Bids (Console-CSV)", "Bidder (Console-Choiceset)"))

  doc.plt = paste0(pth.figure,'check_','line_bidders_diff.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  # 
  
}


check_bidder_unit_on_market <- function(df.hh_choiceset_clean,df.daily_units,
                        df.bids,df.household,df.fm_unit_res,pth.results){
  unit_choiceset <- unique(as.vector(as.matrix(df.hh_choiceset_clean[,c(3:52)])))
  unit_market <- unique(df.daily_units$unitId)
  unit_bids <- unique(df.bids$unit_id)
  
  
  
  df.tmp <- df.bids %>%
    filter(bid.status==1) %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  unit_from_bidder <- unique(df.tmp$fm_unit_id)
  df.tmp_sold_unit <- df.bids %>%
    filter(bid_status==1) %>%
    filter(unit_id %in% unit_from_bidder)
  
  
  num_bidder_unit_in_market <- sum(unit_from_bidder %in% unit_market)
  num_bidder_unit_in_choiceset <- sum(unit_from_bidder %in% unit_choiceset)
  num_bidder_unit_in_bids <- sum(unit_from_bidder %in% unit_bids)
  num_bidder_unit_sold <- length(unique(df.tmp_sold_unit$unit_id))
  
  num_remain_unit = length(unit_market) - num_bidder_unit_in_market
  
  num_successful_bids = sum(df.bids$bid.status==1)
  
  doc.txt = paste0(pth.results,'summary.txt')
  file.conn <- file(doc.txt,'w')
  writeLines(c(paste0(length(unit_market),' units on market'),
               paste0(length(unit_choiceset),' units in choiceset'),
               paste0(length(unit_bids),' units in bids.csv'),
               paste0(num_successful_bids,' successful bids in bids.csv'),
               paste0(length(unit_from_bidder),' unique successful bidders,',
                      ' among their units'),
               paste0(' ',num_bidder_unit_in_market,
                      ' of their units seen on market, ',
                      ' ',num_bidder_unit_in_choiceset,
                      ' in choiceset, ',
                      ' ',num_bidder_unit_in_bids,
                      ' in bids.csv, and',
                      ' ',num_bidder_unit_sold,
                      ' sold')
               ), file.conn)
  
  
  bidder.success = df.tmp
  
  df.tmp <- df.bids %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    filter(!(bidder_id %in% bidder.success$bidder_id)) %>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  unit_from_bidder <- unique(df.tmp$fm_unit_id)
  df.tmp_sold_unit <- df.bids %>%
    filter(bid_status==1) %>%
    filter(unit_id %in% unit_from_bidder)
  
  
  num_bidder_unit_in_market <- sum(unit_from_bidder %in% unit_market)
  num_bidder_unit_in_choiceset <- sum(unit_from_bidder %in% unit_choiceset)
  num_bidder_unit_in_bids <- sum(unit_from_bidder %in% unit_bids)
  num_bidder_unit_sold <- nrow(df.tmp_sold_unit)
  
  num_remain_unit = num_remain_unit - num_bidder_unit_in_market
  
  
  
  
  
  
  writeLines(c(
               paste0(length(unit_from_bidder),' unsuccessful bidders,',
                      ' among their units'),
               paste0(' ',num_bidder_unit_in_market,
                      ' of their units seen on market, ',
                      ' ',num_bidder_unit_in_choiceset,
                      ' in choiceset ',
                      ' ',num_bidder_unit_in_bids,
                      ' in bids.csv, and',
                      ' ',num_bidder_unit_sold,
                      ' sold'),
               paste0(num_remain_unit,' units not belong to any bidder.')
                ), file.conn)
  
  
  ## checking pending units
  
  df.tmp <- df.fm_unit_res %>%
    filter(sale_from_date>as.Date('2012-01-01'))
  
  pending_unit_ids <- unique(df.tmp$fm_unit_id)
  
  num_in_bids <- sum(unit_bids %in% pending_unit_ids)
  num_in_choiceset <- sum(unit_choiceset %in% pending_unit_ids)
  num_in_markets <- sum(unit_market %in% pending_unit_ids)
  
  
  
  df.tmp <- df.bids %>%
    filter(unit_id %in% pending_unit_ids & bid_status==1)
  num_in_successful <- length(unique(df.tmp$unit_id))
  
  last_day = tail(unique(df.bids$bid_timestamp),n=1) + as.Date('2012-01-01')
  
  df.tmp <- df.tmp %>%
    mutate(occupancy_from_date = as.Date(occupancy_from_date)) %>%
    filter(occupancy_from_date > last_day)
  
  num_remains = nrow(df.tmp)
  
  writeLines(c(
    paste0('For those units which sale_from_dates are after 2012-01-01'),
    paste0(' ',num_in_markets,
           ' seen on market, ',
           ' ',num_in_choiceset,
           ' in choiceset ',
           ' ',num_in_bids,
           ' in bids.csv, and',
           ' ',num_in_successful,
           ' sold,'),
    paste0(
      'In the end of simulation, ',num_remains, ' successful bidders are waiting to',
      ' move into pending units (occupancy>', last_day,')')
  ), file.conn)
  
  
  close(file.conn)
}



check_day_on_market_bidders <- function(df.bids,df.hh_choiceset_clean,
                                pth.figure){
  
  #number of days un successful bidders stay on market 
  
  df.tmp <-  df.bids %>%
    filter(bid_status == 1)
  
  
  successful_bidder_ids <- unique(df.tmp$bidder_id)
  all_bidder_ids <- unique(df.bids$bidder_id)
  unsuccessful_bidder_ids <- all_bidder_ids[!(all_bidder_ids %in% 
                                                successful_bidder_ids)]
  
  
  df.tmp <- df.hh_choiceset_clean %>%
    select(day,householdId) %>%
    group_by(householdId) %>%
    summarise(num_days = n()) %>%
    filter(householdId %in% unsuccessful_bidder_ids)
  
  #mean(df.tmp$num_days)
  
  ggplot(df.tmp) + 
    geom_histogram(aes(num_days),bins = 200) +
    ggtitle(paste0("Number of Days on Market(UnSuccessful Bidders)")) +
    xlab('Number of Days') + ylab('Frequecy')
  doc.plt = paste0(pth.figure,'histogram_days_on_market_unsuccess_bidders.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  df.tmp <- df.hh_choiceset_clean %>%
    select(day,householdId) %>%
    group_by(householdId) %>%
    summarise(num_days = n()) %>%
    filter(householdId %in% successful_bidder_ids)
  
  ggplot(df.tmp) + 
    geom_histogram(aes(num_days),bins = 200) +
    ggtitle(paste0("Number of Days on Market(Successful Bidders)")) +
    xlab('Number of Days') + ylab('Frequecy')
  doc.plt = paste0(pth.figure,'histogram_days_on_market_success_bidders.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  
  
  df.tmp <- df.hh_choiceset_clean %>%
    select(day,householdId) %>%
    group_by(householdId) %>%
    summarise(num_days = n()) %>%
    filter(!(householdId %in% all_bidder_ids))
  
  ggplot(df.tmp) + 
    geom_histogram(aes(num_days),bins = 200) +
    ggtitle(paste0("Number of Days on Market (Bidders who didn't bid once)")) +
    xlab('Number of Days') + ylab('Frequecy')
  doc.plt = paste0(pth.figure,'histogram_days_on_market_other_bidders.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  
  
  
  # df.tmp <- df.bids %>%
  #   group_by(bidder_id) %>%
  #   summarise(num_days = n()) %>%
  #   filter(bidder_id %in% unsuccessful_bidder_ids)
  # 
  # 
  # ggplot(df.tmp) + 
  #   geom_histogram(aes(num_days),bins = 200) +
  #   ggtitle(paste0("Number of Days on Market(Unsold Units)")) +
  #   xlab('Number of Days') + ylab('Frequecy')
  # doc.plt = paste0(pth.figure,'histogram_days_on_market_unsold_units.png')
  # ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
}


check_day_on_market_units <- function(df.bids,df.daily_units,
                                pth.figure,df.household){
  df.tmp <- df.bids %>%
    filter(bid.status==1) %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  unit_from_bidder <- unique(df.tmp$fm_unit_id)
  
  df.unique_units <- df.daily_units %>%
    mutate(uni_id = unitId*100000+day) %>%
    filter(!duplicated(uni_id)) %>%
    select(-uni_id)
  
  
  df.tmp <- df.unique_units %>%
    filter(unitId %in% unit_from_bidder) %>%
    group_by(unitId) %>%
    summarise(num_days = n()) %>%
    arrange(num_days)
  
  ggplot(df.tmp) + 
    geom_histogram(aes(num_days),bins = 200) +
    ggtitle(paste0("Number of Days on Market(Successful Bidders' Units)")) +
    xlab('Number of Days') + ylab('Frequecy')
  doc.plt = paste0(pth.figure,'histogram_days_on_market_bidder_units.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  
  
  df.tmp <- df.bids %>%
    filter(bid.status==1)
  
  units_sold <- df.tmp$unit_id
  
  
  df.tmp <- df.unique_units %>%
    filter(!(unitId %in% c(units_sold))) %>%
    group_by(unitId) %>%
    summarise(num_days = n()) %>%
    arrange(num_days)
  
  ggplot(df.tmp) + 
    geom_histogram(aes(num_days),bins = 200) +
    ggtitle(paste0("Number of Days on Market(Unsold Units)")) +
    xlab('Number of Days') + ylab('Frequecy')
  doc.plt = paste0(pth.figure,'histogram_days_on_market_unsold_units.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
}

check_daily_units_by_seller <- function(df.bids,df.daily_units,df.hh_choiceset_clean,
                                      pth.figure,df.household){
  df.tmp <- df.bids %>%
    filter(bid.status==1) %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  unit_from_bidder <- unique(df.tmp$fm_unit_id)
  
  
  df.tmp_units <- df.daily_units %>%
    group_by(day) %>%
    summarise(num_daily = length(unique(unitId)),
              num_from_bidders = sum(unique(unitId) %in% unit_from_bidder)
              )%>%
    mutate(num_initial_vacant = num_daily - num_from_bidders)
  
  ggplot(df.tmp_units) + 
    geom_line(aes(x = day, y = num_daily,colour="All")) +
    geom_line(aes(x = day, y = num_from_bidders,colour="From Bidders")) +
    geom_line(aes(x = day, y = num_initial_vacant,colour="From Initial Vacant")) +
    ggtitle(paste0("Number of Units on Market (from bidders versus initial vacant)")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Number of Units")
  # labels = c("Console", "dailyHousingMarketUnits.csv",
  #                        "HHChoiceSet.csv"))
  
  doc.plt = paste0(pth.figure,'check_','line_units_seller_type.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  
  
  num_choiceset = c()
  num_from_choiceset_bidder = c()
  days <- unique(df.hh_choiceset_clean$day)
  for(i in days){
    #i = days[1]
    df.one_day <- df.hh_choiceset_clean %>%
      filter(day == i) %>%
      select(-day,-householdId)
    unit_ids <- unique(as.vector(as.matrix(df.one_day)))
    unit_ids <- unit_ids[!is.na(unit_ids)]
    num_choiceset <- c(num_choiceset,length(unit_ids))
    num_from_choiceset_bidder <- c(num_from_choiceset_bidder,
                                   sum(unit_ids %in% unit_from_bidder))
  }
  df.tmp <- data.frame(days,num_choiceset,num_from_choiceset_bidder)
  df.tmp_units <- df.tmp %>%
    rename(day=days) %>%
    mutate(num_from_choiceset_initial = num_choiceset - num_from_choiceset_bidder) %>%
    merge(df.tmp_units,by='day',all=T)
  
  
  ggplot(df.tmp_units) + 
    geom_line(aes(x = day, y = num_choiceset,colour="All")) +
    geom_line(aes(x = day, y = num_from_choiceset_bidder,colour="From Bidders")) +
    geom_line(aes(x = day, y = num_from_choiceset_initial,colour="From Initial Vacant")) +
    ggtitle(paste0("Number of Units in Choiceset (from bidders versus initial vacant)")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Number of Units")
  # labels = c("Console", "dailyHousingMarketUnits.csv",
  #                        "HHChoiceSet.csv"))
  
  doc.plt = paste0(pth.figure,'check_','line_units_seller_type_choiceset.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  
    
  
    
  
  df.tmp_units <- df.bids %>%
    group_by(bid_timestamp) %>%
    summarise(num_bid_units = length(unique(unit_id)),
              num_bid_units_from_bidder = sum(unique(unit_id) 
                                              %in% unit_from_bidder)) %>%
    mutate(num_bid_units_from_initial = num_bid_units - num_bid_units_from_bidder) %>%
    rename(day = bid_timestamp) %>%
    merge(df.tmp_units,by='day',all=T)
  
  ggplot(df.tmp_units) + 
    geom_line(aes(x = day, y = num_bid_units,colour="All")) +
    geom_line(aes(x = day, y = num_bid_units_from_bidder,colour="From Bidders")) +
    geom_line(aes(x = day, y = num_bid_units_from_initial,colour="From Initial Vacant")) +
    ggtitle(paste0("Number of Units in bids.csv (from bidders versus initial vacant)")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Number of Units")
  # labels = c("Console", "dailyHousingMarketUnits.csv",
  #                        "HHChoiceSet.csv"))
  
  doc.plt = paste0(pth.figure,'check_','line_units_seller_type_in_bids.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  
  
  
  df.tmp_units
}

check_pending_units <- function(){
  
  
  unit_choiceset <- unique(as.vector(as.matrix(df.hh_choiceset_clean[,c(3:52)])))
  unit_market <- unique(df.daily_units$unitId)
  unit_bids <- unique(df.bids$unit_id)
  
  df.tmp <- df.fm_unit_res %>%
    filter(sale_from_date>as.Date('2012-01-01'))
  
  pending_unit_ids <- unique(df.tmp$fm_unit_id)
  
  num_in_bids <- sum(unit_bids %in% pending_unit_ids)
  num_in_choiceset <- sum(unit_choiceset %in% pending_unit_ids)
  num_in_markets <- sum(unit_market %in% pending_unit_ids)
  
  df.tmp <- df.bids %>%
    filter(unit_id %in% pending_unit_ids & bid_status==1)
  num_in_successful <- length(unique(df.tmp$unit_id))
  
  last_day = tail(unique(df.bids$bid_timestamp),n=1)
  
  df.tmp <- df.tmp %>%
    mutate(occupancy_from_date = as.Date(occupancy_from_date)) %>%
    filter(occupancy_from_date > as.Date('2012-01-01')+last_day)
    
  
    
}



check_multiple_bids <- function(){
  df.tmp <- df.bids %>%
    filter(bid_status==1) %>%
    group_by(bidder_id) %>%
    summarise(success_time = n()) %>%
    filter(success_time>1)
  
  df.bidder_multi_success <- df.bids %>%
    filter(bidder_id %in% df.tmp$bidder_id) %>% filter(bid_status==1)
  
  
  
  df.tmp <- df.bids %>%
    filter(bid_status==1) %>%
    group_by(unit_id) %>%
    summarise(sold_time = n()) %>%
    filter(sold_time>1)
  
  df.unit_multi_sold <- df.bids %>%
    filter(unit_id %in% df.tmp$unit_id) %>% filter(bid_status==1)
  
  out.file = paste0(pth.results,'units_sold_multiple_times.csv')
  write.csv(df.unit_multi_sold,out.file)
  
  ### check units of those unsuccessful bidders
  
  df.tmp <- df.bids %>%
    filter(bid.status==1) %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  bidder.success = df.tmp
  
  df.tmp <- df.bids %>%
    select(bidder_id) %>%
    filter(!duplicated(bidder_id))%>%
    filter(!(bidder_id %in% bidder.success$bidder_id)) %>%
    merge(df.household[,c('hh_id','fm_unit_id')],
          by = 1,all.x=T)
  
  unit_from_bidder_unsuccess <- unique(df.tmp$fm_unit_id)
    
  df.tmp <- df.bids %>%
    filter(bid_status==1) %>%
    filter(unit_id %in% unit_from_bidder_unsuccess)
  
  
  
  df.tmp_ <- df.bids %>%
    filter(bidder_id == 97775)
  #97775,117238
  df.tmp_ <- df.bids %>%
    filter(unit_id == 313429)
   
}


