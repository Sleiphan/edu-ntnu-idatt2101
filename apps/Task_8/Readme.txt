Programmet må kompileres med en C++-kompilator.

Det brukes ved å gi tre argumenter: 2 filer og et flagg som sier om det skal komprimeres eller dekomprimeres. Den første filen er kildefilen som skal komprimeres, og den andre filen er der resultatet skal skrives til. Flagget kan være enten -c (for komprimering) eller -d (for de-komprimering).

Programmet benytter Huffmann-kompresjon. Det skiller seg ut i måten det lagrer symboltabellen, ved å bruke 0-bits som ordre for "gå dypere ned i treet" og 1-bits som "vi har nådd en løv-node!". Denne metoden står bedre beskrevet i kunngjøringen "om å lage Huffmann-trær".