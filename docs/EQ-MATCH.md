# LOVE EQ — Réparation interface et EQ MATCH v1

## Réparation des vignettes et des textes

Cause précise des huit vignettes vides : les libellés étaient dessinés dans le panneau parent, alors que chaque `TextButton` enfant peint son propre fond opaque au-dessus. Le correctif remplace les huit boutons standards par des `BandTileButton` qui dessinent eux-mêmes numéro, type de filtre, fréquence et gain, avec leur vraie couleur et l'état de sélection. Toutes les données viennent de l'APVTS, les boutons restent cliquables.

La typographie de l'éditeur est unifiée sur Arial, avec des chaînes françaises C++17 UTF-8 explicites. Le contrôle automatique rejette les caractères de remplacement et les séquences de texte corrompu. La vérification visuelle sur macOS reste nécessaire : un test textuel seul ne prouve pas le bon rendu d'une fonte.

Les sous-vues TIME LENS, LOVE INTENT, RELATIONSHIP EQ et EQ MATCH masquent leurs commandes incompatibles, sans supprimer les huit bandes ni modifier le moteur audio simplement lors de l'ouverture d'une sous-vue. Contrôles de rectangles pour les modes au format de fenêtre fixe 1160 × 760.

## EQ MATCH : fonctionnement réel

1. Insérer LOVE EQ sur la piste à traiter.
2. Cliquer sur **EQ MATCH** (bouton dans la deuxième ligne, près des onglets).
3. Faire lire la source dont on veut reproduire le caractère sonore, puis cliquer sur **CAPTURER RÉF**. La capture dure 2 secondes environ, à 24 relevés spectraux par seconde.
4. Faire lire ensuite la source à égaliser, puis cliquer sur **CAPTURER SOURCE** pendant sa lecture.
5. Cliquer sur **PROPOSER** : le moteur analyse les deux captures mesurées dans 6 bandes logarithmiques centrées à 85, 220, 560, 1450, 3600 et 8500 Hz. L'écart de niveau global est supprimé, afin de comparer la balance fréquentielle et non le volume.
6. Cliquer sur **B ÉCOUTER** pour entendre les six filtres. Le bouton bascule sur **A ORIGINAL**, qui restaure l'état précédent.
7. Modifier le gain de chaque bande et l'intensité de l'effet, puis **CONSERVER** ou **ANNULER**. Les réglages conservés font partie du projet et des presets USER.

Le traitement utilise six filtres Bell indépendants, réellement branchés au DSP, à correction initiale limitée à ±6 dB par bande, avec interpolation des gains et du mix pendant 30 ms. Il n'écrase aucune des huit bandes classiques, ni TIME LENS, ni LOVE INTENT. Aucune proposition n'est générée à partir d'un signal silencieux. Pas d'allocation ni de verrou supplémentaire dans le callback audio.

## Limites et validations

- Il s'agit d'une première approximation en six bandes, pas d'une reconstruction complète de la courbe de référence avec résolution arbitraire.
- Les deux références sont capturées **à travers la même instance pendant la lecture** ; la capture directe depuis un fichier WAV, le stockage persistant des spectres et le routage inter-instances ne sont pas encore inclus.
- Les propositions sont fondées sur le spectre réel et ne sont pas une recommandation perceptuelle universelle ; un mixage doit être évalué à niveau comparé.
- Les changements en cours de capture sont réalisés dans l'éditeur, pas dans le callback audio. Fermer le panneau avant validation rétablit le son original.
- Le build macOS et le contrôle visuel des caractères français, dimensions de tous les boutons et états de chaque mode sont obligatoires. Un build vert ne garantit pas à lui seul un affichage correct dans un DAW.
