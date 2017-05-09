CalDebevec <- read.table("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalDebevec.csv", header = TRUE)
my.d <- max(CalDebevec)
plot(CalDebevec$r, main="Calibrate Debevec",col="red", xlim=c(0,255), type="l", ylim=c(0,my.d), xlab = "Mesured intencity", ylab = "Calibrated intensity")
lines(CalDebevec$g ,col="green")
lines(CalDebevec$b ,col="blue")

CalRobertson <- read.table("/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/CalRobertson.csv", header = TRUE)
my.r <- max(CalRobertson)
plot(CalRobertson$r, main="Calibrate Robertson",col="red", xlim=c(0,255),type="l", ylim=c(0,my.r), xlab = "Mesured intencity", ylab = "Calibrated intensity")
lines(CalRobertson$g ,col="green")
lines(CalRobertson$b ,col="blue")

