# LOVE EQ - interface de reference

La capture LIBERATION EQ fournie par l'utilisateur est la reference visuelle pour LOVE EQ.

## Interface implementee

- En-tete avec signature LOVE EQ / by Liberty Music Software, recherche et menu de 100 presets, comparaison A/B, annuler et refaire, bypass.
- Trois onglets visibles : PARAMETRIQUE et DYNAMIQUE pour les fonctions existantes. GRAPHIQUE affiche un etat desactive tant que son DSP n'est pas livre : aucune fonction simulée.
- Grand graphe central avec huit points colores, reponse EQ et spectres PRE et POST.
- Nouveaux outils reels de spectre : FIGER et MAINTIEN PIC, traites hors du thread audio.
- Vumetres d'entree/sortie a droite du graphe.
- Huit tuiles de bande alignees, indiquant type de filtre, frequence et gain en temps reel.
- Panneau inferieur en trois sections : FILTRE, DYNAMIQUE et EQ MATCH. EQ MATCH reste clairement indique comme en developpement.
- Entree/sortie dans le bandeau inferieur.

## Protection de l'affichage

Taille de travail 1160 x 760 px sans ascenseur ni redimensionnement fragile.
Champs numeriques separes des libelles. Aucun texte de fantaisie ni glyphes decoratifs devant remplacer une lettre.
La source C++ est verifiee en UTF-8 par un test CI. Les accents francais sont encodes avec des escapes Unicode dans les libelles critiques pour eviter le mojibake lors d'editions multiplateformes.

## Reste a valider sur poste reel

Captures macOS dans un DAW et en Standalone pour verifier la taille sur un ecran 13 pouces.
Le test statique de texte et la compilation ne suffisent pas a prouver l'absence de chevauchement visible sur tous les ecrans.
Signature/notarisation, analyse audio comparative et tests dans les hotes restent necessaires avant une release.
