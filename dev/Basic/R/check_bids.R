clear_df <- function(){
  #change the value of x to any value to force a refresh
  x = 0
}

load_csv <- function(results,filename){
  df = read.csv(paste0(results,filename), stringsAsFactors = F)
}

set_result_folder <- function(folder){
  pth.results = folder
}

create_figure_folder <- function(pth.results){
  pth.figure = paste0(pth.results,'figure/')
  dir.create((pth.figure))
  pth.figure
}

get_bids_file <- function(pth.results) {
  doc.bids = "bids.csv"
  
  
  
  df.bids = read.csv(paste0(pth.results,doc.bids),stringsAsFactors = F)
  colnames(df.bids)  = gsub("\\.", "_", colnames(df.bids))
  
  
  
  df.bids$unit.type = factor(df.bids$type_id)
  df.bids$bid.status = factor(df.bids$bid_status)
  
  df.bids$asking.hp.ratio = df.bids$asking_price/df.bids$hedonicprice
  df.bids$target.hp.ratio = df.bids$target_price/df.bids$hedonicprice
  df.bids$target.asking.ratio = df.bids$target_price/df.bids$asking_price
  
  df.bids$wtp.hp.ratio = df.bids$bidder_wtp/df.bids$hedonicprice
  df.bids$wtp_er.asking.ratio = df.bids$bidder_wp_wp_error/df.bids$asking_price
  
  df.bids$bid.hp.ratio = df.bids$bid_value/df.bids$hedonicprice
  df.bids$bid.wtp.ratio = df.bids$bid_value/df.bids$bidder_wp_wp_error
  df.bids$bid.asking.ratio = df.bids$bid_value/df.bids$asking_price
  df.bids$bid.target.ratio = df.bids$bid_value/df.bids$target_price
  
  df.bids
}

get_successful_bids <- function(df.bids){
  df.bids.success = df.bids[df.bids$bid_status==1,]
}

get_bids_units <- function(df.bids){
  df.bids.units = df.bids[!duplicated(df.bids$unit_id),]
}


draw_bids_generic <- function(df.bids,pth.figure,pre.file='market_'){
  ##------ bids of each day
  bids_accept = tapply(df.bids$bid_status, df.bids$bid_timestamp, FUN=sum)
  bids_all = tapply(df.bids$bid_status, df.bids$bid_timestamp, FUN=length)
  df.results = data.frame(day = as.integer(names(bids_all)), all = bids_all, bids = bids_accept,
                          bids_cum = cumsum(bids_accept))
  
  ggplot(df.results) +
    geom_line(aes(x = day, y = all,colour="darkblue")) +
    geom_line(aes(x = day, y = bids,colour="red")) +
    #geom_line(aes(x = day, y = bids_cum,colour="lightskyblue")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Bids", labels = c("All", "Success"))
  
  doc.plt = paste0(pth.figure,pre.file,'line_bids_count.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  
  ##------ number of bids on each unit type
  ggplot(df.bids) +
    geom_bar(aes(x=unit.type,fill=bid.status)) + ggtitle("Number of Bid") +
    xlab('Unit Type') + ylab('Number') 
  doc.plt = paste0(pth.figure,pre.file,'bar_bids_num.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  ##------ number of bids of each unit type
  df.bids.success = df.bids[df.bids$bid_status==1,]
  df.bids.success$unit.type = as.numeric(df.bids.success$unit.type)
  df.bids.success$main_unit_type = NA
  df.bids.success[df.bids.success$unit.type <= 6 |
                    df.bids.success$unit.type == 65,
                  'main_unit_type' ] = 'HDB'
  df.bids.success[df.bids.success$unit.type >= 7 &
                    df.bids.success$unit.type <= 11,
                  'main_unit_type' ] = 'Apt'
  df.bids.success[df.bids.success$unit.type >= 12 &
                    df.bids.success$unit.type <= 16,
                  'main_unit_type' ] = 'Condo'
  df.bids.success[df.bids.success$unit.type <= 17 &
                    df.bids.success$unit.type >=31,
                  'main_unit_type' ] = 'Terrace, (Semi-) Detached,'
  df.bids.success[df.bids.success$unit.type >= 32 &
                    df.bids.success$unit.type <= 36,
                  'main_unit_type' ] = 'EC'
  
  
  ggplot(df.bids.success)+ 
    geom_bar(aes(x=main_unit_type,fill=bid.status)) + ggtitle("Number of Sale") +
    xlab('Unit Type') + ylab('Number') 
  doc.plt = paste0(pth.figure,pre.file,'bar_sale_num.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  
  ##------ histgram number of bids of each unit
  cnt.units = data.frame(table(df.bids$unit_id))
  cnt.units.large = cnt.units[cnt.units$Freq>100,]
  
  ggplot(cnt.units.large) +
    geom_histogram(aes(x=Freq),bins=100) + ggtitle("Histogram of Bids Count > 100") +
    xlab('Count') + ylab('Frequency') 
  doc.plt = paste0(pth.figure,pre.file,'hist_bids_count.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  ##------ number of freelance seller and normal seller
  count_unique <-function(x,threshold=0){
    length(unique(x[x>threshold]))
  }
  
  all_seller = tapply(df.bids$seller_id, df.bids$bid_timestamp, FUN=count_unique)
  freelance_seller = tapply(df.bids$seller_id, df.bids$bid_timestamp, FUN=count_unique,
                            threshold=9000000)
  df.results = data.frame(day = as.integer(names(all_seller)), all = all_seller, 
                          hh = all_seller-freelance_seller,
                          freelance = freelance_seller )
  
  ggplot(df.results) +
    geom_line(aes(x = day, y = freelance,colour="darkblue")) +
    geom_line(aes(x = day, y = hh,colour="red")) +
    #geom_line(aes(x = day, y = bids_cum,colour="lightskyblue")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Seller", labels = c("Freelance", "Household"))
  doc.plt = paste0(pth.figure,pre.file,'line_seller_num.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  
}



draw_seller <- function(df.bids,pth.figure,ext.title,ext.file,pre.file='seller_'){
  #------- Seller
  #------- distribution of HP
  ggplot(df.bids) +
    geom_histogram(aes(x=hedonicprice),bins = 100) +
    facet_wrap(~unit.type, ncol=6, scales = "free") + 
    xlab('HP') + ylab('Count') + ggtitle(paste0("HP ~ Unit Type -",ext.title))
  doc.plt = paste0(pth.figure,pre.file,'hist_hp_unit_type_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  
  ##------ ratio of asking/hp
  ggplot(df.bids, aes(x=unit.type,y=asking.hp.ratio)) +
    geom_boxplot() + ggtitle(paste0("Asking/HP - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio') 
  doc.plt = paste0(pth.figure,pre.file,'boxplot_asking_hp_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  ##------ ratio of target/hp
  ggplot(df.bids, aes(x=unit.type,y=target.hp.ratio)) +
    geom_boxplot() + ggtitle(paste0("Target/HP - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_target_hp_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  ##------ ratio of target/asking
  ggplot(df.bids, aes(x=unit.type,y=target.asking.ratio)) +
    geom_boxplot() + ggtitle(paste0("Target/Asking - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_target_asking_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
}


draw_buyer <- function(df.bids,pth.figure,ext.title,ext.file,pre.file='buyer_'){
  #------- Buyer
  #------- distribution of WTP
  ggplot(df.bids) +
    geom_histogram(aes(x=bidder_wtp),bins = 100) +
    facet_wrap(~unit.type, ncol=6, scales = "free") + 
    xlab('WTP') + ylab('Count') + ggtitle(paste0("WTP ~ Unit Type -",ext.title))
  doc.plt = paste0(pth.figure,pre.file,'hist_wtp_unit_type_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  
  ##------ ratio of wtp/hp
  ggplot(df.bids, aes(x=unit.type,y=wtp.hp.ratio)) +
    geom_boxplot() + ggtitle(paste0("WTP/HP - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_wtp_hp_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  ##------ ratio of bid/hp
  ggplot(df.bids, aes(x=unit.type,y=bid.hp.ratio)) +
    geom_boxplot() + ggtitle(paste0("Bid/HP - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_bid_hp_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  ##------ ratio of bid/WTP
  ggplot(df.bids, aes(x=unit.type,y=bid.wtp.ratio)) +
    geom_boxplot() + ggtitle(paste0("Bid/WTP - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_bid_wtp_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
  ##------ ratio of bid/asking
  ggplot(df.bids, aes(x=unit.type,y=bid.asking.ratio)) +
    geom_boxplot() + ggtitle(paste0("Bid/Asking - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_bid_asking_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  
  ##------ ratio of wtp/asking
  ggplot(df.bids, aes(x=unit.type,y=wtp_er.asking.ratio)) +
    geom_boxplot() + ggtitle(paste0("WTP/Asking - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_wtp_asking_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4)
  
  
  ##------ ratio of bid/target
  ggplot(df.bids, aes(x=unit.type,y=bid.target.ratio)) +
    geom_boxplot() + ggtitle(paste0("Bid/Target - ",ext.title)) +
    xlab('Unit Type') + ylab('Ratio')
  doc.plt = paste0(pth.figure,pre.file,'boxplot_bid_target_ratio_',ext.file,'.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 4 )
  
}



draw_market_from_console <- function(pth.results){
  folder = pth.results
  pth.figure = paste0(pth.results,'/figure/')

  filenames = dir(pth.results)
  pattern = 'simlt.+\\.txt'
  
  filepath = filenames[grepl(pattern,filenames)]
  
  con = file(paste0(folder,filepath), "r")
  lines = readLines(con)
  close(con)
  
  pattern = '.*HUnits:\\s*(\\d+).*'
  lines = lines[grepl(pattern,lines)]
  num_hunits = as.integer(gsub(pattern,"\\1",lines))
  
  pattern = '.*Bidders:\\s*(\\d+).*'
  num_bidders = as.integer(gsub(pattern,"\\1",lines))
  
  pattern = '.*Accepted:\\s*(\\d+).*'
  num_accept = as.integer(gsub(pattern,"\\1",lines))
  
  pattern = '.*Bids:\\s*(\\d+).*'
  num_bids = as.integer(gsub(pattern,"\\1",lines))
  
  
  
  day = 1:length(num_bids)-1
  
  df.results <- data.frame(day,num_hunits,num_bidders,num_bids,num_accept)
  
  ggplot(df.results) +
    geom_line(aes(x = day, y = num_bids,colour="darkblue")) +
    geom_line(aes(x = day, y = num_accept,colour="red")) +
    #geom_line(aes(x = day, y = bids_cum,colour="lightskyblue")) +
    xlab('Day') + ylab('Count')  +
    scale_color_discrete(name = "Bids", labels = c("All", "Success"))
  
  doc.plt = paste0(folder,'console_','line_bids_count.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3 )
  
  df.results
}



