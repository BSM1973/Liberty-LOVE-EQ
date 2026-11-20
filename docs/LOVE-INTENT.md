# LOVE INTENT — Égalisation par intention musicale (v1)

## Finalité
Un assistant de propositions, et non un égaliseur automatique opaque. L'utilisateur choisit une intention, lance l'analyse d'une source réellement en lecture, compare trois variantes, écoute A/B, ajuste les bandes et décide explicitement si la proposition doit être conservée.

## Intentions disponibles
- **Plus clair sans agressivité** : alléger les bas-médiums, améliorer modérément la présence et limiter les aigus durs.
- **Préserver la chaleur de la voix** : respecter les graves utiles, détendre les bas-médiums trop chargés et calmer la zone de dureté.
- **Adoucir les aigus** : conserver du corps et réduire une présence trop incisive sans enlever toute l'énergie haute.
- **Définir la basse** : stabiliser les graves, alléger la boue du bas-médium et conserver l'articulation.

## Proposition et transparence
- ANALYSER mesure le spectre d'entrée de LOVE EQ à l'aide du véritable analyseur 2048 points et calcule un profil moyen dans cinq zones : grave, corps, présence, dureté, air.
- Le moteur « LOVE INTENT » est volontairement déterministe et testable : pas de « réseau d'IA » supposé, pas d'inférence mensongère de tonalité ou d'instrument.
- Trois variantes (DOUX, ÉQUILIBRÉ, AFFIRMÉ), chacune avec trois filtres Bell dont le gain proposé est strictement limité à ±3 dB. La version plus affirmée applique une dose plus importante, tout en conservant la même philosophie.
- Dans les trois panneaux, la zone ciblée et le rôle musical de la correction sont décrits en français. Les courbes originales sont affichées en cyan, ambre et violet ; elles restent indicatives tant que l'utilisateur ne déclenche pas l'écoute B.
- En présence de silence ou d'énergie trop faible, **aucune proposition** n'est présentée.
- Les suggestions sont un point de départ : la qualité musicale doit être évaluée à l'écoute, et dépend des instruments et du contexte.

## Audio, A/B et sécurité
- Trois filtres Bell **indépendants des huit bandes classiques**, de TIME LENS et de RELATIONSHIP EQ ; aucune des huit bandes d'origine n'est réécrite pour créer les suggestions.
- Le traitement est désactivé par défaut, avec gain lissé, intensité réglable de 0 à 100 % et fondu sur 30 ms lors de l'activation/désactivation.
- A ORIGINAL : rétablit tous les paramètres LOVE INTENT enregistrés avant l'analyse (activation, dose, fréquences, gains et Q).
- B PROPOSITION : charge uniquement les trois bandes de la variante et les active pour la préécoute. Les bandes deviennent éditables via les sliders et l'APVTS, avec automatisation et sauvegarde dans les projets.
- CONSERVER : confirme explicitement la variante B actuellement écoutée ; sans ce clic, la proposition n'est jamais considérée comme validée.
- ANNULER : restaure intégralement les paramètres initiaux. Fermer la vue ou l'éditeur avec une proposition non confirmée restaure également la base.
- Les comparaisons A/B de LOVE INTENT sont indépendantes des commandes A/B globales du plugin, lesquelles continuent à gérer les états globaux de LOVE EQ.
- Tous les changements sont effectués sur le thread d'interface ; le callback audio ne crée ni objet dynamique ni verrou.
- Le DSP et ses paramètres (y compris CONSERVER) peuvent être stockés dans un preset USER avec la bibliothèque existante.

## Interface
- Bouton LOVE INTENT dans la barre sous le titre, sans recouvrir les commandes existantes.
- Ligne d'édition : choix de l'intention, ANALYSER, trois variantes, A ORIGINAL / B PROPOSITION, CONSERVER / ANNULER.
- Ligne d'intensité et message de statut ; sous le spectre, trois panneaux dotés d'une couleur et de sliders fréquence, gain, Q.
- Les trois courbes se superposent au graphe sans masquer ou remplacer la réponse des bandes d'origine ; aucune nouvelle barre de défilement et fenêtre fixe 1160 × 760.
- Textes français directement en UTF-8 (avec anglais au choix), menus sombres déjà présents dans LOVE EQ.

## Limites v1
- La mesure repose sur un instantané de spectre 2048 points ; elle ne reconnaît ni les notes ni les syllabes et ne mesure pas la référence de loudness à long terme.
- Il ne s'agit pas d'une EQ Match automatique, d'un moteur de normalisation LUFS ni d'une décision perceptuelle prouvée.
- Les courbes sont des heuristiques prudentes, non un jugement universel sur les sons ; la valeur exacte d'une correction dépend de la prise.
- Des écoutes sur voix, guitare, piano, basse et mix complet, plus un contrôle des clics à l'activation et une comparaison à niveau égal, restent nécessaires avant de prétendre à une qualité professionnelle définitive.

## Vérifications
- Tests C++17 sans JUCE : analyse de cinq bandes, rejet du silence et des NaN, limites ±3 dB, ordonnancement des trois variantes, cohérence musicale minimale des propositions.
- Validation statique de la liaison UI et des paramètres réels, de la restauration A/B, des accents et des collisions géométriques à 1160 × 760.
- CI macOS : compilation AU, VST3 et Standalone + artefact ZIP requis après validation.
