Experiments in Numenta-style hierarchical temporal memory, i.e. a boiled-down model of the neocortex.

See: http://www.numenta.com

Equally, experiments in OpenGL, using it to visualize the state of the HTM.

Re: sparse distributed HTM:

Conceptually, I think of HTM as having similarities to Git: Git encodes the state of a project
into a signature that can be used to uniquely identify and recall that state. If the signature
space is large enough and the encoding algorithm is sufficiently "hashy", you never need to
worry about collisions in the signature space. HTM takes a theoretically unlimited (but
_practically_ bounded) input space, generates a "signature", and then reinforces just the
subset of a grossly overbuilt neural network that generates the signature, and temporal
derivitives of it... Winner-take-all mechanisms, inhibition mechanisms, (micro/macro) columnar
organization, etc. are all variations on a "reinforce the signature-subset of an overbuilt
neural network" theme.

The neocortex probably isn't as good as git/RSA at generating unique signatures... The brain is
eager to make connections, sometimes to a fault: optical/aural/etc. illusions can be
attributed to over-eager spatial or temporal associations... On the other hand, these over-
eager associations can also be an engine of generalization, inference, and imagination.

Departures of my implementation from the "canonical" HTM:

* I process non-binary inputs by simulating sensors which are assigned a sensitivity threshold;
They'll only fire if the input exceeds the sensitivity threshold, i.e. some cells will fire only
for "bright" inputs, while others will fire for both "bright" _and_ "dim" inputs. Intuitively
it seems biologically plausible; and regardless it should be amenable to the HTM "algorithm".

* My implementation simulates, right or wrong, an "afterimage" effect by storing activation
states in the high bit of a word that is right shifted (i.e. it decays with a half-life)
with each time step of the network. This ties in with the sensitivity-assigned sensor concept
for feed-fwd inter-region interactions.

* I add a new state beyond "active" and "predicting": "Imagining". It's a layer above predicting
that propagates predictions based on predictions, that importantly does not impact the network-
reinforcing mechanisms. These "imaginations" do decay with the same half-life mechanism described
above, to avoid a "prediction/imagination storm" where all of the cells feed each other in an
endless, incoherent loop.  My hope is that when my HTM is implemented and tuned properly, that the
"imagination" layer will behave just like its name implies, producing an internally-generated
"train-of-thought" that can be back-propagated down through the regions towards the sensory layers;
Ideally where "hallucinations" could be directly visualized.

Re: OpenGL

If nothing else, the visualization component of this little project can be of use to someone, it
does make pretty pictures. :)

Ultimately, I'd like to implement the HTM critical paths in GLSL.


This is a work-in-progress, and probably always will be...
Comments in the code are about as sparse as this README.
