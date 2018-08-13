

check_screen_probability <- function(df.screenprobs,df.hh_choiceset_clean,
                                     df.daily_units,df.choice_set_daily_units,
                                     df.unit_alternative,pth.figure){
  
  df.daily_units_alternative <- df.unit_alternative %>%
    select(fm_unit_id,map_id) %>%
    merge(df.daily_units,by.x='fm_unit_id',by.y='unitId')
  
  tb.daily_alter = table(c(df.daily_units_alternative$day,rep(99999,216)),
                         c(df.daily_units_alternative$map_id,c(1:216)))
  tb.daily_alter = tb.daily_alter[c(1:(nrow(tb.daily_alter)-1)),]
  
  df.daily_alter = as.data.frame.matrix(tb.daily_alter)
  
  
  df.daily_choiceset_althernative <- df.unit_alternative %>%
    select(fm_unit_id,map_id) %>%
    merge(df.choice_set_daily_units,by.x='fm_unit_id',by.y='unit_id')
  
  tb.daily_cs_alter = table(c(df.daily_choiceset_althernative$day,rep(99999,216)),
                         c(df.daily_choiceset_althernative$map_id,c(1:216)))
  tb.daily_cs_alter = tb.daily_cs_alter[c(1:(nrow(tb.daily_cs_alter)-1)),]
  
  df.daily_cs_alter = as.data.frame.matrix(tb.daily_cs_alter)
  
  
  df.screenprobs_tmp <- df.screenprobs %>%
    filter(!duplicated(householdId))
  
  df.day_households <- df.hh_choiceset_clean %>%
    select(day,householdId)
  

  days = max(df.daily_units$day)
  df.screen_probs = df.daily_alter[0,]
  df.market_alter = df.daily_alter[0,]
  df.choiceset_alter = df.daily_alter[0,]
  for(i in 1:days){
    #i = 207
    start_day = i
    end_day = i+1
    
    df.tmp <- df.day_households %>%
      filter(day>=start_day & day<end_day)
    
    df.probs <- df.screenprobs_tmp %>%
      filter(householdId %in% df.tmp$householdId)
    
    mtx.probs <- as.matrix(df.probs[,c(2:217)])
    screen_probs <- colMeans(mtx.probs)
    df.screen_probs[i,] = screen_probs
    
    mtx.alter <- as.matrix(df.daily_alter[c((start_day+1):end_day),])
    market_alter <- colMeans(mtx.alter)
    market_alter <- market_alter/sum(market_alter)
    df.market_alter[i,] = market_alter
    
    mtx.alter <- as.matrix(df.daily_cs_alter[c((start_day+1):end_day),])
    choiceset_alter <- colMeans(mtx.alter)
    choiceset_alter <- choiceset_alter/sum(choiceset_alter)
    df.choiceset_alter[i,] = choiceset_alter
    
    
    #print(c(i,sum(tmp_probs),sum(tmp_alter)))
    #results = c(results,cor(screen_probs,choiceset_alter))
    #results = c(results,sum(tmp_probs*tmp_alter))
  }
  
  cor1 = c()
  cor2 = c()
  cor3 = c()
  for(i in 1:days){
    #i = 78
    screen_probs = as.numeric(df.screen_probs[i,])
    choiceset_alter = as.numeric(df.choiceset_alter[i,])
    market_alter = as.numeric(df.market_alter[i,])
    
    cor1 = c(cor1,cor(screen_probs,choiceset_alter))
    cor2 = c(cor2,cor(screen_probs,market_alter))
    cor3 = c(cor3,cor(choiceset_alter,market_alter))
  }
  
  df.cs_unit_daily = df.choice_set_daily_units %>%
    group_by(day) %>%
    summarise(num_unit = length(unique(unit_id)))
  
  
  
  
  df.tmp <- data.frame(cor1,cor2,cor3) %>%
    mutate(days = 1:n())
  
  ggplot(df.tmp) +
    geom_line(aes(x = days, y = cor1,colour="screening v.s. choiceset")) +
    geom_line(aes(x = days, y = cor2,colour="screening v.s. market")) +
    geom_line(aes(x = days, y = cor3,colour="choiceset v.s. market")) +
    xlab('days') + ylab('correlation')  +
    scale_color_discrete(name = "")
  
  doc.plt = paste0(pth.figure,'lines_','correlation_screening_alternative.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  
  
  
  
  # pth.tmp = paste0(pth.figure,'/screen/')
  # dir.create((pth.tmp))
  # 
  # cutoff_prob = sort(cor1)[10]
  # for(i in 1:days){
  #   # if (cor1[i]>cutoff_prob){
  #   #   next
  #   # }
  #   screen_probs = as.numeric(df.screen_probs[i,])
  #   choiceset_alter = as.numeric(df.choiceset_alter[i,])
  #   market_alter = as.numeric(df.market_alter[i,])
  # 
  #   df.tmp = data.frame(screen_probs,choiceset_alter,market_alter) %>%
  #     mutate(alternative = 1:216)
  #   ggplot(df.tmp) +
  #     geom_line(aes(x = alternative, y = screen_probs,colour="screening probs")) +
  #     geom_line(aes(x = alternative, y = choiceset_alter,colour="chcoiceset")) +
  #     geom_line(aes(x = alternative, y = market_alter,colour="market")) +
  #     xlab('alternative') + ylab('probability')  +
  #     ylim(-0.002, 0.065) +
  #     scale_color_discrete(name = "")
  # 
  #   doc.plt = paste0(pth.figure,'screen/lines_','screening_probs_day_',i,'.png')
  #   ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  # 
  # }
  
  gini_market = c()
  gini_screen = c()
  for(i in 1:days){
    screen_probs = as.numeric(df.screen_probs[i,])
    choiceset_alter = as.numeric(df.choiceset_alter[i,])
    market_alter = as.numeric(df.market_alter[i,])
    
    gini_market = c(gini_market,ineq(market_alter,type="Gini"))
    gini_screen = c(gini_screen,ineq(screen_probs,type="Gini"))
  }
  
  
  df.tmp <- data.frame(gini_market,gini_screen) %>%
    mutate(days = 1:n())
  
  ggplot(df.tmp) +
    geom_line(aes(x = days, y = gini_market,colour="market gini")) +
    geom_line(aes(x = days, y = gini_screen,colour="screening gini")) +
    xlab('days') + ylab('gini')  +
    scale_color_discrete(name = "")
  
  doc.plt = paste0(pth.figure,'lines_','gini_screening_alternative.png')
  ggsave(doc.plt, width = 10, height = 5, units = "cm", scale = 3)
  
  df.tmp
  # df.one_day <- df.hh_choiceset_clean %>%
  #   filter(day == 79)
  # 
  # df.one_day <- df.choice_set_daily_units %>%
  #   filter(day == 79)
  # length(unique(df.one_day$unit_id))
  
}



units_in_choice_set_each_day <- function(df.hh_choiceset_clean){
  vt.days = c()
  vt.units = c()
  days <- unique(df.hh_choiceset_clean$day)
  for(i in days){
    #i = days[1]
    df.one_day <- df.hh_choiceset_clean %>%
      filter(day == i) %>%
      select(-day,-householdId)
    unit_ids <- as.vector(as.matrix(df.one_day))
    unit_ids <- unit_ids[!is.na(unit_ids)]
    vt.days = c(vt.days,rep(i,length(unit_ids)))
    vt.units = c(vt.units,unit_ids)
  }
  df.tmp <- data.frame(vt.days,vt.units) %>%
    rename(day = vt.days, unit_id = vt.units)
}

















