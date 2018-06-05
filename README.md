
# Innhold

1.  [Introduksjon](#org0f3d3ff)
2.  [Systemkrav](#org72434d7)
3.  [Installasjon](#org9c3c645)
4.  [Bruk](#orgab75adf)
5.  [Rapporter](#orgfdad6e4)
6.  [Loggfiler](#org4d2389f)
7.  [Konfigurasjonsfil](#orgb3c44ca)
8.  [Ønsker, feil og mangler](#orgf6b65ae)
9.  [Hvordan rapportere feil](#orgdf9db9e)
10. [Historikk](#org98f9770)
    1.  [2018.06.01 innsyn-v1.0.0](#orgceeefcf)
    2.  [2018.04.13 innsyn-v1.0.0-rc1](#org6a53c4e)
    3.  [2018.02.05 innsyn-v1.0.0-beta2](#org34c2305)
    4.  [2018.01.18 innsyn-v1.0.0-beta1](#orgbe2026f)


# Innhold

1.  [Introduksjon](#org0f3d3ff)
2.  [Systemkrav](#org72434d7)
3.  [Installasjon](#org9c3c645)
4.  [Bruk](#orgab75adf)
5.  [Rapporter](#orgfdad6e4)
6.  [Loggfiler](#org4d2389f)
7.  [Konfigurasjonsfil](#orgb3c44ca)
8.  [Ønsker, feil og mangler](#orgf6b65ae)
9.  [Hvordan rapportere feil](#orgdf9db9e)
10. [Historikk](#org98f9770)
    1.  [2018.06.01 innsyn-v1.0.0](#orgceeefcf)
    2.  [2018.04.13 innsyn-v1.0.0-rc1](#org6a53c4e)
    3.  [2018.02.05 innsyn-v1.0.0-beta2](#org34c2305)
    4.  [2018.01.18 innsyn-v1.0.0-beta1](#orgbe2026f)


<a id="org0f3d3ff"></a>

# Introduksjon

KDRS Innsyn er en applikasjon for å åpne og søke i NOARK-5 uttrekk. Applikasjonen validerer ikke uttrekkene utover å sjekke at vedleggene finnes.  Applikasjonen er datadrevet så langt det er mulig, dvs den har ikke innebygde referanser til NOARK-5 nøkkelord eller XML knagger. Istedenfor kontrollerers utsendet ved hjelp av innstillinger i konfigurasjonsfilen.


<a id="org72434d7"></a>

# Systemkrav

Applikasjonen er testet på Windows 10 64bit og Windows 7 64bit.


<a id="org9c3c645"></a>

# Installasjon

1.  Pakk ut *innsyn-v1.0.0.zip*.
2.  Indeksering av vedlegg kan bruke en del plass, rediger derfor eventuelt
    *REPORTS\_DIR* i *insight.conf* til å peke på en katalog med mye ledig
    diskplass.
3.  Språk i brukergrensesnittet kan settes ved å endre *LANGUAGE* i 
    *insight.conf*. Tilgjenelige språk er *engelsk*, *bokmål* og *nynorsk*. Standard språk er satt til bokmål. Vær obs på at språk ikke bør endres etter at programmet er tatt i bruk.


<a id="orgab75adf"></a>

# Bruk

![img](/Users/ole/dev/cmu_112/trunk/archivator/client_tools/insight/screenshot.png)

1.  Kjør *insight.exe* fra katalogen *innsyn-v1.0.0*.
2.  Brukergrensesnittet har fire hovedelementer:
    1.  **Nodetre**: Viser alle elementene i uttrekket i en trestrukture etter en import. 
        Hver node i treet tilsvarer en XML knagg. Nodetreet viser også tidligere 
        innlastede uttrekk. Disse kan lastes inn på nytt eller slettes
        ved å bruke knappene **Importer** og **Slett** i nodeinformasjon ruten.
    2.  **Nodeinformasjon**: Viser detaljert informasjon om en node. Enkelte 
        elementer har kanapper som lar deg utføre handlinger på informasjonen. 
        For eksempel har nodeelement som er vedlegg en **Vis** kanpp som lar 
        brukeren vise vedlegget med standard visningsverktøy som er registrert 
        for filformatet i operativsystemet.
    3.  **Søk**: Lar brukeren søke i importerte uttrekk.
    4.  **Søkeresultat**: Liste som kun vises når brukeren gjør ett søk.
3.  Trykk **Importer** for å laste inn ett NOARK-5 uttrekk. Ett lite
    testdatasett ligger under mappen *testdata*.
4.  Innlasting starter, for store uttrekk kan dette ta lang tid, men det er
    mulig å se på XML nodene mens innlastingen pågår.
5.  Når importeringen er ferdig blir brukeren spurt om vedleggene skal
    indekseres. Indekseringen kjører i bakgrunnen og kan ta lang tid avhengig av
    hvor mange vedlegg uttrekket inneholder. Første gang indekseringen kjører
    kan det hende at operativsystemets brannmur spør om godkjenning for
    prosessen *searchd.exe*, godta dette.
6.  Import genererer en rapport i rapport mappen, den kan åpnes fra rot noden
    i treet.
7.  Programmet har to hovedfunksjoner:
    1.  **Søk**: Fritekst søk på innholdet i nodene. Når søket starter
        så kommer en liste med søketreff frem. Dobbel-klikk å ett element i
        listen for å navigere til elementet i node treet. Søketreffene kan
        krysses av ved å trykke **Velg** knappen. Ved å klikke på **+** knappen ved siden av **Velg** knappen kan brukeren spesifisere hvordan søket skal fungere:
        a) **Vedlegg**: Vises kun hvis indekseringen er ferdig. Hvis den er avkrysset skjer søket mot vedleggene.
        b) **Skill mellom små og store bokstaver**: Hvis den ikke er avkrysset vil søketeksten *kdrs* gi treff på både *KDRS*, *kDrs* og *kdrs*. Hvis den er avkrysset vil den kun gi treff på *kdrs*.
        c) **Inkuder noder**: Lister alle unike tre node navn. Noder som ikke er avkrysset blir ikke tatt med i søkeresultatet.
        For å skjule de avanserte søkevalgene, trykk på **-** knappen.
    2.  **Eksporter**: Vil generere en rapport bestående av de nodene som er valgt
        i node treet. Rapporten kan lagres til disk eller sendes som vedlegg på
        e-post. Vær klar over at skjermet informasjon ikke skal sendes på vanlig e-post.


<a id="orgfdad6e4"></a>

# Rapporter

Etter import genereres en PDF-rapport i rapport katalogen, bestemt av
*REPORTS\_DIR* i *insight.conf*. Rapportene legges i en katalogstruktur på
formatet: *REPORTS\_DIR\\åååå\MM\DD\TTMMSS\\*.  I rapport katalogen
legges også loggfiler fra indekseringen og indekserings databasen.


<a id="org4d2389f"></a>

# Loggfiler

-   **innsyn-v1.0.0\insight.log**: Applikasjonslogg.
-   **innsyn-v1.0.0\insight.dmp**: Genereres hvis applikasjonen krasjer.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\attachments.log**: Logg over konvertering av vedlegg til søkbare tekstfiler.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\indexer.log**: Logg fra indekserings applikasjonen som genererer indekserings databasen.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\sphinx\test1\searchd.log**: Logg fra søkemotoren.


<a id="orgb3c44ca"></a>

# Konfigurasjonsfil

Konfigurasjonsfilen heter *insight.conf*. Alle nøkler er dokumentert i konfigurasjonsfilen. Ved endring må
applikasjonen restartes.


<a id="orgf6b65ae"></a>

# Ønsker, feil og mangler

-   [IKAMR-20180515-01] Du burde kanskje hatt en boks med informasjon eller på en eller annen måte oversiktlig vist hvilke uttrekk du faktisk har åpen no som kan søkes i?
-   [IKAMR-20180515-02] Det samme gjelder treff i søk, hvordan vet jeg hvilken av uttrekkene søket fant sine treff i?
-   [IKAMR-20180514-01] Etter å ha lukket uttrekk: Her står "Import ble avbrutt av brukeren", mener programmet at jeg har avbrutt fordi jeg klikket LUKK?
-   [IKAMR-20180514-01] Info etter at indeksering er ferdig bør vises i en egen dialogboks. (Infobar nederst har ikke nok plass til å vise all informasjon.)
-   [IKAMR-20180514-01] Begrensing som gjør at nodetree med over 200.000 elementer ikke kan minimeres (kollapses) er problematisk når flere samtidige uttrekk er lastet inn.
-   [IKAMR-20180425-01] Savner FRITEKST søk, dvs. at om jeg skriver Ola Normann og der er egne xml elemetner for <foranvn>Ol</fornavn> og <etternavn>Normann</Normann> så vil jeg få et treff i fritekstsøk. Full fritekstsøk bør være en avkrysningsboks, fordi avgrenset søk UTEN full fritekst vil være normalen i Noark 5-sammenheng og ellers? Så hvis jeg får mitt ønske gjennom får vi både punkt 1. Avansert søk feltnivå i forrige epost, OG 4. FRITEKST søk som nevnt her.
-   [IKAMR-20180424-01] Mer avansert søk på feltnivå (som <xml elementnavn 1>=<verdi 1> OG / ELLER <xml elementavn 2>=<verdi 2> osv.
-   [IKAMR-20180424-02] Når jeg har fått et resulat, f. eks. 9 treff på et etternavn, så finner jeg ikke en enkel måte å velge "merk alle" på. Høyreklikk velg alle f. eks.? Da kan jeg bla gjennom til venstre de som er avkrysste og ev. ta vekk de som ikke er aktuelle
-   [IKAMR-20180424-03] Vis kun valgte (avkryssede). Noe som er meget relevant i et Noark 5-utvalg også etter søk.
-   [KDRS-20180315-04] Se på framtidig mulighet til å koble seg til KS sin FIKS plattform for sending av informasjon til innbygger
-   [KDRS-20180315-06] Se om det er mulig å få med antall funne rader – og ikke bare vist
-   [IKAVA-20180308-01] Søk knappen burde muligens vært tilknyttet søkeresultat-området i GUI, og at knappen oppe ved søk beholdt "Søk" teksten, for nytt søk.
-   [IKARO-20180125-03] Avkrysning i sjekkboksene (nodene) bør ta med seg underliggende noder.
-   [IKAMR-20180120-06] "Lagre som snarvei" en sti og enkel gjenbruke de lagrede snarveier.
-   [IKAMR-20180120-09] Vise antall søketreff.
-   [IKAMR-20180120-10] Vise antall treff for forskjellige kategorier.
-   For store node tre så kan kollapsing og så ekspandering av noder ta
    lang tid. Dette er derfor slått av for nodetre med flere enn 200.000
    noder.
-   Får ikke advarsel hvis indeksering starter og det er igjen lite plass
    på disken.


<a id="orgdf9db9e"></a>

# Hvordan rapportere feil

ole.liabo@piql.com / 454 69 123
Ved feilrapportering legg ved loggfiler og ved programkrasj *insight.dmp* hvis den finnes.


<a id="org98f9770"></a>

# Historikk


<a id="orgceeefcf"></a>

## 2018.06.01 innsyn-v1.0.0

-   [IKAMR-20180425-02] BUG på norske tegn i søk? Hvis her er norske tegn si vurker ikke "Skill mellom små og store bokstaver" som IKKE avkrysset

dvs. <fornavn>TORBJØRN</fornavn> vil ikke få treff når jeg søker med "Torbjørn".


<a id="org6a53c4e"></a>

## 2018.04.13 innsyn-v1.0.0-rc1

-   [IKAVA-20180122-03] Inkluder dokumentnavn i nodetree visningen.
-   [IKARO-20180223-02] Bør kunne slette ved å høyreklikke på root node i treet.
-   [IKAMR-20180120-04] Enkel brukerveiledning.
-   [IKARO-20180125-06] Help evt. kort brukerveiledning.
-   [KDRS-20180315-01] Statistikk på hvor mange dokumenter som ikke inneholder tekst, og hvor mange med tekst.
-   [KDRS-20180315-02] Advarsel på tidsbruk ved indeksering av pdf-er
-   [KDRS-20180315-03] Advarsel i forbindelse med «send på e-post»
-   [IKAVA-20180308-02] Søk skal ikke se forskjell på små og store bokstaver.
-   [KDRS-20180315-07] Neste knapp i søkeresultat.
-   [KDRS-20180315-05] Muligheter for utvidet søk – med å kunne avgrense søkeobjektene – slik at en kan raffinere søket bedre
-   [KDRS-20180315-08] Telle antall valge noder.
-   [IKARO-20180223-03] Importer kanppen får teksten Rapport.
-   [IKAVA-20180122-01] Mindre info ved søk, filtrere vekk "uninteressante" felter i søkeresultat.
-   [IKARO-20180125-01] På Public 360 4.1 krasjet arkivstruktur.xml og arkivuttrekk.xml.
-   [IKARO-20180125-02] På Visma VSA 2.3.5 krasjet det på arkivuttrekk.xml.
-   [IKAMR-20180120-12] Krasj ved import.
-   [IKARO-20180223-01] Jeg har en rekke tidligere kjøringer listet i hovedvinduet når jeg starter opp. Disse er tomme. Hvis jeg prøver å «Åpne» en av disse, så krasjer Insight. Det blir ikke lagt en insight.dmp. Dette skjer hver gang.


<a id="org34c2305"></a>

## 2018.02.05 innsyn-v1.0.0-beta2

Oppdateringer basert på tilbakemeldinger fra IKA-MR, IKA-VA og IKA-Rogaland.

-   [IKAMR-20180120-01] Standard filnavn "arkivstruktur.xml".
-   [IKAMR-20180120-08] Mulighet til å laste inn allerede importerte uttrekk.
-   [IKAMR-20180120-13] Flere åpne arkiv samtidig, søk på tvers av arkiv.
-   [IKAMR-20180120-07] Rutiner for å "rydde" opp data.
-   [IKAMR-20180120-03] Flere avkryssingsmuligheter i nodetree: "Velg alle underliggende", "Fjern alle underliggende"
-   [IKARO-20180125-04] Burde huske siste Import og Eksport-mappe.
-   [IKAMR-20180120-05] Huske sist brukte Eksport eller Import mappe.
-   [IKARO-20180125-05] About box.
-   [IKAMR-20180120-11] Sortering på kategori.
-   [IKAMR-20180120-02] Dato format i rapport: "Lørdag 20.01.2018 19:32:59".
-   [IKAVA-20180122-02] Unngå at lokasjon brekker over to rader, evt la bruker styre bredden.
-   [PIQL-20180122-01] Konfigurasjonsfil bedre dokumentert.
-   [PIQL-20180122-02] Generere filen insight.dmp ved programkrasj.
-   [PIQL-20180122-03] Viser størrelsen på vedlegg ved import.

eta


<a id="orgbe2026f"></a>

## 2018.01.18 innsyn-v1.0.0-beta1

Første versjon levert til beta test.

