# Bugs signalés (test du 28/10/2026)

- [x] Coffres qui s'ouvrent à l'infini (SpriteObj::CurrentFrame comptait depuis 0)
- [x] Sol invisible : les colonnes BGLayer étaient dessinées devant les murs
- [x] Portraits : tableau dans le cadre, tremblement, poursuite en tournant, tirs (à vérifier en jeu)
- [x] Canon : tirait du mauvais côté une fois retourné vers le joueur
- [x] Lustres posés par terre (Weighted=False ignoré)
- [ ] Chandeliers, chandelles, chaises : le coup est reçu (Break) mais l'image reste entière
- [x] Étagères : plateformes (PhysicsObj Collidable, dessus seulement)
- [ ] Rampes : font des marches au lieu de pentes
- [ ] Saut non variable (relâcher le bouton doit couper le saut)
- [ ] Bas + saut sur une plateforme : descendre à travers
- [ ] Bas + attaque en l'air : attaque vers le bas
- [ ] Ennemis qui ne font pas de dégâts (ex. le fantôme à l'œil)
- [ ] Après un coup, le héros glisse lentement sur le côté
- [ ] À la mort, le héros disparaît
- [ ] Le héros dérive parfois sur le côté, même immobile
