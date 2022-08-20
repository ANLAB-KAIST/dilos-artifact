*** sp.pl.old	Sun Aug  9 12:58:01 2015
--- sp.pl.new	Sun Aug  9 12:59:08 2015
***************
*** 44,65 ****
  	$VERBOSE = 0;
  
  	$RM_FLAG = "";  # used for RunUsePath
  
  
! 	$TIMEZONE = "TZ_UNDEF";
! 
! 	######
! 	# note:  TZ on solaris is set to point to a file in /usr/share/lib/zoneinfo.
! 	# e.g:  /usr/share/lib/zoneinfo/US/Pacific
! 	# date +%Z will return the old form, such as PST or PDT.
! 	######
! 	my($tmp) = `sh -c "date +%Z"`;
! 	$status = ($? >> 8);
! 	if ($status == 0){
! 		chomp $tmp;
! 		$TIMEZONE = $tmp;
! 	}
! 
  }
  
  sub check_env
--- 44,58 ----
  	$VERBOSE = 0;
  
  	$RM_FLAG = "";  # used for RunUsePath
+ }
  
+ use POSIX;
+ my $TIMEZONE = undef;
  
! sub init_timezone
! {
! 	#we only initialize if we need to.
! 	$TIMEZONE = strftime("%Z", localtime()), "\n" unless($TIMEZONE);
  }
  
  sub check_env
***************
*** 212,217 ****
--- 205,211 ----
  sub GetDateStr
  	# Return the current date as if you ran `date`
  {
+ 	&init_timezone();
  	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdt) = localtime();
  	$date = (Sun,Mon,Tue,Wed,Thu,Fri,Sat)[$wday];
  	$date .= " ";
***************
*** 225,230 ****
--- 219,225 ----
  sub GetDateStrFromUnixTime
  {
  	local($tme) = @_;
+ 	&init_timezone();
  	($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdt) = localtime($tme);
  	$date = (Sun,Mon,Tue,Wed,Thu,Fri,Sat)[$wday];
  	$date .= " ";
