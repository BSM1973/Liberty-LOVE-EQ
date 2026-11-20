# LOVE EQ - HARMONIC DNA (première intégration)

## Objectif
Protéger l'identité musicale d'une source monophonique en identifiant les composantes qui forment sa série harmonique et en séparant les pics qui ne semblent pas y appartenir. L'outil n'affirme jamais qu'un pic isolé est nécessairement mauvais.

## Fonctionnement livré
- Historique de 8192 échantillons avant égalisation, actualisé par le moteur audio temps réel sans allocations supplémentaires dans le callback.
- Analyse FFT 8192 points effectuée uniquement dans l'éditeur (non dans le thread audio), avec fenêtre de Hann et nouvelle estimation environ 3 fois par seconde.
- Recherche de fréquences fondamentales candidates à partir de plusieurs pics et de leurs rapports harmoniques : possibilité de reconnaître une fondamentale même lorsque sa première harmonique est moins forte que ses partiels.
- Estimation de confiance. Quand le signal est silencieux, trop peu harmonique ou trop complexe, aucune fondamentale fiable n'est proposée.
- Marquage turquoise des harmoniques associées à la fondamentale sur le spectre. Un pic non aligné apparaît dans la zone HARMONIC DNA comme « à examiner » et est indiqué en rose sur le spectre.
- Affichage de la note, de la fréquence, d'une estimation de confiance et du nombre de partiels trouvés ; deux langues, français par défaut.
- CIBLER : place la bande sélectionnée sur le pic à examiner, Q=6 et gain=0 dB, sans aucune coupe ni amplification.
- ATTÉNUER : ne devient disponible qu'après CIBLER et applique -2 dB après clic explicite. Historique Undo/Redo du DAW conservé.
- En désactivant HARMONIC DNA, le panneau précédent EQ MATCH non fonctionnel réapparaît, sans modifier le son.
- Fonction active seulement dans les vues paramétrique et dynamique ; le mode graphique continue de fonctionner sans superposition.

## Limites connues
- Il s'agit d'une première estimation spectrale de source MONOPHONIQUE à fondamentale stable. Ne pas se fier au pourcentage comme à une probabilité statistique calibrée.
- Sur un accord de guitare, plusieurs instruments, des frappes percussives et certaines voix soufflées, le résultat peut être erroné : aucune correction automatique n'est appliquée.
- Un « pic à examiner » peut être un partiel inharmonique voulu, une note supplémentaire, une consonne, un bruit mécanique intéressant ou une coloration naturelle.
- L'algorithme ne réécrit pas encore automatiquement la réponse des filtres pour « protéger » les partiels. Il protège les décisions de l'utilisateur en évitant les corrections non demandées et en affichant les composantes à préserver.
- Prochaines validations audio nécessaires : guitares graves en Mi2, voix avec vibrato, transitoires, synthèse à forte résonance, polyphonie et mesure de charge CPU.
- Le graphique ne remplace pas les écoutes A/B et ne garantit pas une identification parfaite des harmoniques.

## Tests
- Synthèses de séries harmoniques pour guitare (Mi2, 82,4 Hz), voix (Sol3, 196 Hz) et synthétiseur (La4, 440 Hz).
- Détection de pic inharmonique clairement identifié à 920 Hz.
- Absence de détection automatique sur silence, bruit large bande uniforme et onde sinusoïdale sans série harmonique.
- Compilation Linux et macOS et artefacts AU/VST3/Standalone dans GitHub Actions.

## Règles UI conservées
- Fenêtre 1160 x 760, aucune barre de défilement, aucun chevauchement voulu.
- Caractères français réels en UTF-8 et police compatible, pas de pictogramme pour remplacer les accents.
- Couleurs de bandes conservées (bande 5 cyan) ; cette extension occupe le panneau jusque-là réservé à EQ MATCH.
