chemin <-"/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/etudeImages/"
pathsec = "essai5_WR/"

t0 <- "window_Rob"

fic <-paste ("Calibrage_",t0,"_List.txt",sep="")
titre <-paste ("Calibrage image ",t0,sep="")

debevec <- paste(chemin,pathsec,fic,"CalDebevec.csv",sep="")
CalDebevec <- read.table(debevec, header = TRUE)
my.d <- max(CalDebevec)
t1 <- paste(titre," méthode Debevec Protocol 1",sep="")
plot(CalDebevec$r, main=t1,col="red", xlim=c(0,255), type="l", ylim=c(0,my.d), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalDebevec$g ,col="green")
lines(CalDebevec$b ,col="blue")

robertson<-paste(chemin,pathsec,fic,"CalRotertson.csv",sep="")
CalRobertson <- read.table(robertson, header = TRUE)
my.r <- max(CalRobertson)
t2 <- paste(titre," méthode Robertson Protocol 1",sep="")
plot(CalRobertson$r, main=t2,col="red", xlim=c(0,255),type="l", ylim=c(0,my.r), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalRobertson$g ,col="green")
lines(CalRobertson$b ,col="blue")

#video Birgth
fic <-paste ("video_Brigth_",t0,"_List.txt",sep="")
titre <-paste ("Calibrage image ",t0,sep="")

debevec <- paste(chemin,pathsec,fic,"CalDebevec.csv",sep="")
CalDebevec <- read.table(debevec, header = TRUE)
my.d <- max(CalDebevec)
t1 <- paste(titre," méthode Debevec Protocol 2",sep="")
plot(CalDebevec$r, main=t1,col="red", xlim=c(0,255), type="l", ylim=c(0,my.d), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalDebevec$g ,col="green")
lines(CalDebevec$b ,col="blue")

robertson<-paste(chemin,pathsec,fic,"CalRotertson.csv",sep="")
CalRobertson <- read.table(robertson, header = TRUE)
my.r <- max(CalRobertson)
t2 <- paste(titre," méthode Robertson Protocol 2",sep="")
plot(CalRobertson$r, main=t2,col="red", xlim=c(0,255),type="l", ylim=c(0,my.r), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalRobertson$g ,col="green")
lines(CalRobertson$b ,col="blue")

#video Dark
fic <-paste ("video_Dark_",t0,"_List.txt",sep="")
titre <-paste ("Calibrage image ",t0,sep="")

debevec <- paste(chemin,pathsec,fic,"CalDebevec.csv",sep="")
CalDebevec <- read.table(debevec, header = TRUE)
my.d <- max(CalDebevec)
t1 <- paste(titre," méthode Debevec Protocol 3",sep="")
plot(CalDebevec$r, main=t1,col="red", xlim=c(0,255), type="l", ylim=c(0,my.d), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalDebevec$g ,col="green")
lines(CalDebevec$b ,col="blue")

robertson<-paste(chemin,pathsec,fic,"CalRotertson.csv",sep="")
CalRobertson <- read.table(robertson, header = TRUE)
my.r <- max(CalRobertson)
t2 <- paste(titre," méthode Robertson Protocol 3",sep="")
plot(CalRobertson$r, main=t2,col="red", xlim=c(0,255),type="l", ylim=c(0,my.r), xlab = "Valeur du pixel", ylab = "Valeur Calibrée")
lines(CalRobertson$g ,col="green")
lines(CalRobertson$b ,col="blue")
