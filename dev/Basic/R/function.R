

load_table <- function(table_name,schema_name="synpop12"){
  require("RPostgreSQL")

  print("Looking for database credentials in ../private/lt-db.ini")
  con = file("../private/lt-db.ini", "r")
  while ( TRUE ) {
    line = readLines(con, n = 1)
    if ( length(line) == 0 ) {
      break
    }

    if( substr(line, 0,9 ) == "password=")
    {
      pw <- substr(line, 10, 1000000)
      print("password=********")
    }
    else
    {
      print(line)
    }
  }
  close(con)
  
  # loads the PostgreSQL driver
  drv <- dbDriver("PostgreSQL")
  # creates a connection to the postgres database
  # note that "con" will be used later in each connection to the database
  con <- dbConnect(drv, dbname = "simmobility_sg12_2d.1.1",
                   host = "localhost", port = 5432,
                   user = "fmstaff", password = pw)
  
  myTable <- dbReadTable(con, c(schema_name,table_name))
  
  dbDisconnect(con)
  
  myTable
}


link_unit_alternative <- function(df.fm_unit_res,
                                  df.unit_type,
                                  df.alternative,
                                  df.view_building_sla_address){
  df.tmp <- df.view_building_sla_address %>%
    select(fm_building_id,planning_area_name)
  
  df.tmp <- df.fm_unit_res %>%
    select(fm_unit_id,fm_building_id,unit_type) %>%
    merge(df.tmp,by='fm_building_id') %>%
    merge(df.unit_type[,c('id','dwelling_type')], by.x='unit_type',by.y='id')
    
  
  df.tmp1 <- df.alternative %>%
    mutate(planning_area_name=trimws(plan_area_name)) %>%
    select(map_id,planning_area_name,dwelling_type_id) %>%
    rename(dwelling_type = dwelling_type_id)
  
  df.tmp <- df.tmp %>%
    merge(df.tmp1,by=c('planning_area_name','dwelling_type'))
  
}
