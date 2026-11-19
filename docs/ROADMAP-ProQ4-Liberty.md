# LOVE EQ - Étude fonctionnelle de FabFilter Pro-Q 4 et identité Liberty

Références publiques consultées : documentation et page officielle FabFilter Pro-Q 4, novembre 2026.
https://www.fabfilter.com/products/pro-q-4-equalizer-plug-in
https://www.fabfilter.com/help/pro-q/intro/fabfilterproq
https://www.fabfilter.com/help/pro-q/using/spectral-dynamics

Ce document recense des fonctions publiques et des objectifs de conception.
Aucun code, ressource graphique, préréglage ni marque FabFilter n'est repris.

## Socle existant avant cette étape
- Huit bandes paramétriques, traitements dynamiques par bande, routages stéréo/Mid/Side, divers filtres.
- Mode graphique 16 bandes, analyseur FFT PRE/POST, maintien de crête et gel.
- A/B, undo/redo, mémorisation dans la session, bibliothèque de 100 presets plus USER.
- Fenêtre fixe 1160 x 760 (13 pouces), français par défaut, thème sombre Liberty.

## Premier lot ajouté : CURVE LAB
- Bouton DESSIN : trace libre de la courbe avec la souris ; un ajustement logarithmique d'origine Liberty propose jusqu'à huit vrais filtres Bell.
- Les bandes générées sont de véritables paramètres APVTS avec restitution et automatisation par l'hôte. Undo groupé.
- NOTES : repères musicaux sur le graphe ; Alt-glisser pour caler la fréquence de la bande sur un demi-ton tempéré (La4=440 Hz).
- CAPTER PIC : détecte une résonance du spectre PRE et place la bande sélectionnée à cette fréquence, gain à 0 dB pour ne jamais couper ou amplifier à l'insu de l'utilisateur.
- Double clic sur le graphe : place une bande Bell active à l'emplacement demandé.
- Validations Linux/macOS : Curve Lab indépendant du moteur JUCE ; aucun accès disque ou allocation ajouté au callback audio.

Ce premier lot ne prétend pas recréer exactement l'algorithme de courbe de Pro-Q 4 ni inclure toutes ses fonctions.

## Fonctionnalités Pro-Q 4 qui constituent des priorités ultérieures
1. 24 bandes paramétriques maximum et édition multiple de groupes. Vérifier CPU et gestion des modifications sans allocations audio.
2. Spectre de très haute résolution, réglages FFT, surimpressions d'instances, aide au repérage des masquages.
3. Dynamique spectrale : moteur WOLA FFT, limitation fréquentielle par bande, fenêtrage et réconstruction, latence exacte et tests de nullité.
4. Natural Phase et Linear Phase : moteurs DSP séparés, mesures du retard de groupe, latence signalée à l'hôte et comparaisons A/B de pré-échos.
5. Auto Gain mesuré avec intégration temps long et contrôle robuste du niveau, pas seulement égalisation théorique.
6. EQ Match, références mémorisées et pondération d'écoutes, avec une dose d'application réglable.
7. Saturation de caractère indépendante et désactivable : tests anti-aliasing, oversampling et débit CPU.
8. Sidechain filtré par bande, solo auditif, plage de gain 3/6/12/30 dB, règles d'accessibilité.
9. Liste d'instances : support sur Liberty DAW d'abord puis API de communication inter-instances sûre.
10. Prise en charge immersive future conditionnée à un vrai moteur multicanal.

## Identité originale proposée pour LOVE EQ

### LOVE RELATIONSHIP EQ - Égalisation par relation entre pistes
Le travail ne porte plus seulement sur une piste isolée, mais sur le rapport entre une source prioritaire (voix, lead, kick) et un ensemble concurrençant le même espace sonore.
- Afficher les conflits en temps/fréquence dans deux sources.
- Mesurer le masquage pondéré perceptivement et expliquer quelle partie est touchée.
- Créer une correction dynamique limitée, proportionnelle aux collisions et déclenchée uniquement si nécessaire.
- Éviter d'endommager la fondamentale d'un instrument et sa famille d'harmoniques.
- Mode « protéger la voix », « conserver l'attaque », « priorité basse », etc. avec paramètres lisibles et contrôlables.
- Compatible avec les canaux Liberty DAW par routages explicites, puis hôtes tiers via sidechain quand possible.
Cette proposition est un axe original de *workflow*, pas une affirmation d'invention absolue des algorithmes de démasquage existants.

### LOVE HARMONIC COMPASS - Mode musical
Détection des hauteurs stables, visualisation des harmoniques utiles, contraste entre résonances gênantes et composantes voulues.
- Afficher la note et les partiels pertinents d'une piste mélodique.
- Protéger ou renforcer les harmoniques de la note, sans coupe systématique des pics.
- Proposer un nettoyage sensible à la tonalité et à l'instrument.
- Exiger des seuils de confiance et un bouton de validation humain.

### LOVE TIME LENS - Deux dimensions du son
Une courbe d'EQ n'est plus uniquement fréquence -> gain ; ajouter des cibles temporelles transitoire, sustain, résonance et fin de note.
- Égalisation ciblée sur l'attaque sans toucher le corps d'un instrument.
- Contrôles de transition avec latence mesurée et traitement anti-clic.
- Modèle de temps réel protégé contre les artefacts, testé sur batterie et piano.

### LOVE INTENT - Suggestions explicables, jamais une boîte noire
L'utilisateur formule un objectif musical (plus clair sans agressivité, basse plus lisible, moins de résonance sans perdre de chaleur).
Le logiciel propose 2 ou 3 courbes avec des métriques d'énergie, de masquage et de niveau, toujours modifiables et comparables A/B.
Les résultats doivent indiquer leurs hypothèses, leurs limites et permettre le retour instantané à l'original.

## Garde-fous non négociables
- Interface originale Liberty, aucun chevauchement, accents français réels, police vérifiée sur macOS 13 pouces.
- Aucun bouton factice ; les éléments non développés ne s'affichent pas comme opérationnels.
- Paramètres temps réel sûrs, persistance des projets, artefacts après builds verts.
- Pas d'utilisation d'un nom FabFilter dans le code commercial ou le branding de LOVE EQ.
