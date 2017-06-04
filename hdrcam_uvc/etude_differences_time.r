chemin <-"/home/esailly/Documents/projetFinal/hdrcam-master/hdrcam_uvc/etudeImages/"
fic.name <-"recap.txt"
fic.recap <- paste(chemin,fic.name,sep="")
recapitulatif <- read.table(fic.recap, header = TRUE)

id <- c(4:6) 
moyD <- c(
        mean(recapitulatif$Calibrage_Deb[recapitulatif$nbImg == 4]),
        mean(recapitulatif$Calibrage_Deb[recapitulatif$nbImg == 5]),
        mean(recapitulatif$Calibrage_Deb[recapitulatif$nbImg == 6])
        )
moyR <- c(
  mean(recapitulatif$Calibrage_Rob[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Calibrage_Rob[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Calibrage_Rob[recapitulatif$nbImg == 6])
)


plot(id, moyD, main="Temps moyen de calibrage / images traitées",col="red", type="l", ylim=c(1000,3100), xlim=c(4,6),
       xlab = "Nombre d'images", ylab = "Temps en ms")
lines(id,moyR ,col="Blue")
legend("topleft", legend=c("Debevec", "Robertson"),
       col=c("red", "blue"), lty=1, cex=0.8, box.lty=0)

moyMD <- c(
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 6])
)
moyMR <- c(
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 6])
)

moyMM <- c(
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 6])
)

plot(id, moyMD, main="Temps moyen de fusion / images traitées",col="red", type="l", ylim=c(30,120), xlim=c(4,6),
     xlab = "Nombre d'images", ylab = "Temps en ms")
lines(id,moyMR ,col="Blue")
lines(id,moyMM ,col="Green")
legend("topleft", legend=c("Debevec", "Robertson", "Mertens"),
       col=c("red", "blue", "green"), lty=1, cex=0.8, box.lty=0)

moyTD <- c(
  mean(recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 6])
)
moyTR <- c(
  mean(recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 6])
)

moyTM <- c(0,0,0)

plot(id, moyTD, main="Temps moyen de mapping / images traitées",col="red", type="l", ylim=c(-1,50), xlim=c(4,6),
     xlab = "Nombre d'images", ylab = "Temps en ms")
lines(id,moyTR ,col="Blue")
lines(id,moyTM ,col="Green")
legend("topleft", legend=c("Debevec", "Robertson", "Mertens"),
       col=c("red", "blue", "green"), lty=1, cex=0.8, box.lty=0)


moyMTD <- c(
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 4]+recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 5]+recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Merge_Deb[recapitulatif$nbImg == 6]+recapitulatif$Tonemap_Deb[recapitulatif$nbImg == 6])
)
moyMTR <- c(
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 4]+recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 5]+recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Merge_Rob[recapitulatif$nbImg == 6]+recapitulatif$Tonemap_Rob[recapitulatif$nbImg == 6])
)

moyMTM <- c(
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 4]),
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 5]),
  mean(recapitulatif$Total_Mer[recapitulatif$nbImg == 6])
)

plot(id, moyMTD, main="Temps traitement de fusion + mapping / images traitées",col="red", type="l", ylim=c(30,120), xlim=c(4,6),
     xlab = "Nombre d'images", ylab = "Temps en ms")
lines(id,moyMTR ,col="Blue")
lines(id,moyMTM ,col="Green")
legend("topleft", legend=c("Debevec", "Robertson", "Mertens"),
       col=c("red", "blue", "green"), lty=1, cex=0.8, box.lty=0)


