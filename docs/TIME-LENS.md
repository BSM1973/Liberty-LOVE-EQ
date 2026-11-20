# TIME LENS — LOVE EQ : égaliser l'évolution d'un son

## Principe
TIME LENS ajoute à LOVE EQ trois égalisations **indépendantes** qui interviennent à différents moments de la **même enveloppe sonore** : ATTAQUE, CORPS et FIN. Il ne s'agit ni d'une animation ni d'une EQ statique à trois bandes supplémentaires.

Exemple : sur une prise de piano, l'ATTAQUE peut réduire un pic métallique à 3,9 kHz sans enlever la chaleur du CORPS à 280 Hz, puis la FIN peut légèrement renforcer la résonance naturelle autour de 1,1 kHz. Le traitement reste librement réglable ; ces valeurs sont des propositions de départ, pas des mesures universelles.

## Fonctionnalités DSP
- Détection temporelle d'attaque/sustain/fin à partir de deux enveloppes lissées (rapide et lente) sur l'énergie d'entrée.
- Seuil réglable de -65 à -12 dB, durée d'attaque réglable de 5 à 150 ms, durée de fin de 60 à 1200 ms.
- Chacune des trois phases possède ses propres paramètres hôte : fréquence, gain et Q ; filtre Bell réel 2 canaux, utilisé en parallèle avec les deux autres.
- Basculement entre phases avec interpolation sur 12 ms et normalisation des contributions, pour limiter les ruptures et les excès de niveau.
- Modulation uniquement des corrections, jamais remplacement du signal sec : y = x + somme (poids × [filtre(x)-x]).
- Fondu global sur 25 ms à l'activation et à la désactivation, sans latence ajoutée dans le plugin.
- Filtres continuellement alimentés même en mode inactif pour garder des historiques cohérents. Coefficients recalculés périodiquement en réponse aux modifications lissées.
- Enveloppe commune aux deux canaux pour préserver la scène stéréo ; détection d'activité robuste même si les canaux sont en opposition de phase.
- Aucun allocateur, verrou, fichier ou FFT ajouté au callback audio.
- Pas de modification involontaire des huit bandes de LOVE EQ, des seize bandes graphiques ou de Relationship EQ.
- Paramètres enregistrés dans les projets et les presets USER via l'APVTS. Les 100 presets d'usine désactivent TIME LENS lors du chargement.

## Interface
- Bouton TIME LENS en haut de la zone spectrale pour ouvrir la vue d'édition ; le mode d'édition ne change pas l'audio.
- Bouton ACTIVER/DÉSACTIVER dans la vue dédiée, désactivé par défaut sur un projet vierge.
- Frise ATTAQUE → CORPS → FIN qui illumine la phase réellement détectée pendant la lecture.
- Trois panneaux d'égalisation indépendants avec leurs contrôles FRÉQUENCE, GAIN, Q, chacun avec une couleur (ambre/turquoise/violet).
- Bandeau de réglage du seuil, de la durée de l'attaque et de la durée de fin. Les huit vignettes habituelles sont masquées seulement dans cette vue et réapparaissent normalement après fermeture.
- Fenêtre fixe 1160×760, sans ascenseur. Marges, accents français réels, textes FR/EN et contrôles non chevauchés.

## Limites importantes (v1)
- **Ce n'est pas un détecteur polyphonique par note.** Une seule enveloppe globale suit l'ensemble de la piste : sur un accord, un sustain continu ou deux notes simultanées, les phases peuvent se confondre.
- « FIN » signifie une diminution d'énergie selon un seuil relatif, pas un message MIDI Note Off. Sur une source coupée instantanément, il ne reste aucun signal à égaliser.
- Sur un piano legato, une guitare saturée fortement compressée ou une voix très bruitée, la séparation peut être approximative. Les réglages doivent être adaptés à la source.
- Le zéro latence concerne ce moteur : aucune détection future/look-ahead ; l'attaque très brève ne peut pas être modifiée avant d'avoir été détectée.
- Cette version n'affirme pas une phase linéaire ni une séparation spectrale des transitoires. Elle utilise une banque de filtres Bell à phase minimale.
- Validation en écoute A/B sur piano, batterie, guitare et voix indispensable avant d'annoncer une qualité finale.

## Tests automatisés
- Simulation de notes sinusoïdales aux fréquences d'échantillonnage 44,1/48/96 kHz.
- Détection des phases dans l'ordre, avec un délai d'attaque non nul, retour au silence, secondes attaques et gestion des échantillons NaN.
- Test que les poids d'étapes restent finis et normalisés.
- Contrat géométrique de l'interface à 1160×760 pour les commandes de TIME LENS.
- Builds macOS AU, VST3, Standalone et test Linux exigés pour la livraison.
