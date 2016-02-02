;; The Where-Am-I helper functions:
;; originally written by Nick Parlante

(in-package "COMMON-LISP-USER")

;;
;;  the 2-D POINT type  (x and y coordinates)
;;  impelemented as a list length 2
;;
;;  the CIRCLE type (a radius and a center point)
;;  implemented as a list length 2-- first element is the radius and
;;  the second is the center point
;;
;;  For convenience, I do not inist that you treat these as ADT's-
;;  so if you want to use CAR to get the x coordinate, or build you own
;;  circles without going through MAKE-CIRCLE that will be ok.
;;  Lisp does have constructs for doing ADT's right, but we
;;  haven't learned them and there's no point is dealing with them.
;; 
;; POINT functions:
;;  make-pt   create a new point  (NOT make-point which is part of Allegro CL)
;;  x         get the x coordinate of a point
;;  y         get the y coordinate of a point
;;  dist      return the distance between two points
;;
;; CIRCLE functions
;;  make-circle   create a new circle
;;  radius        get the radius of a circle
;;  center        get the center of a circle
;;  intersect     given two circles, returns a list of the points
;;                of intersection for those circles.
;;                For the purposes of this program I have
;;                bastardized the definition of 'intersect' a little
;;                to give better results when the measurements are
;;                inexact.  Don't worry about that, just use the points
;;                returned.  Someone who is interested in Math or Geometry
;;                might be interested to see how I compute the intersection.

(defun make-pt(x y) (list x y))
(defun x (pt) (car pt))
(defun y (pt) (second pt))
(defun dist (pt1 pt2) 
  (let ((dx (- (x pt1) (x pt2)))
        (dy (- (y pt1) (y pt2))))
    (sqrt (+ (* dx dx) (* dy dy)))))


;;2D vector operations - used by the intersection function
;;vectors are a lot like points.  You won't need these.
(defun add (v1 v2)
     (list (+ (car v1) (car v2))
           (+ (second v1) (second v2))))

(defun sub (v1 v2)
     (list (- (car v1) (car v2))
           (- (second v1) (second v2))))

(defun len(v)
  (sqrt (+ (* (car v) (car v)) (* (second v) (second v)))))

(defun normalize (v)
  (scale (list (- (second v)) (car v)) (/ (len v))))

(defun scale (v factor)
  (list (* (car v) factor) (* (second v) factor)))


;;Circle  operations
(defun make-circle(radius center) (list radius center))
(defun radius(circle) (car circle))
(defun center(circle) (second circle))

;;Return a list of the points of intersection of the two circles.
;;The circles may not have the same center point
(defun intersect(circle1 circle2)
  (if (equal (center circle1) (center circle2))
  (error "Intersect cannont handle circles with the same center point.")
  (let* ((c1 (center circle1))
           (r1 (radius circle1))
           (c2 (center circle2))
           (r2 (radius circle2))
           (d (dist c1 c2)))
      ;;first check to see if the circles are too far apart to intersect,
      ;;or if one circle is within another.
      (if
        (or (> d (+ r1 r2)) (> r1 (+ d r2)) (> r2 (+ d r1)))
        ;;if there is no real intersection, use the closest tangent points on each
        ;;circle.  This is the bastardization above.
        (list 
     (add c1 (scale (sub c2 c1) (/ r1 d)))  ;c1-> towards c2
     (add c2 (scale (sub c1 c2) (/ r2 d)))) ;c2-> towards c1
    ;;otherwise the circles intersect normally, and I did some hairy
    ;;geometry to show that the following computes the two points
    ;;of intersection.
    (let* ((r12 (* r1 r1))
           (r22 (* r2 r2))
           (d2 (* d d))
           (d1 (/ (+ r12 (- r22) d2) 2 d))
           (h (sqrt (- r12 (* d1 d1))))
           (towards (scale (sub c2 c1) (/ d1 d))) ;;vector c1->c2
           (perp (scale (normalize towards) h)))
      (list (add c1 (add towards perp))
            (add c1 (add towards (scale perp -1)))))))))

;;my own intersection test code
;;(setf c0 (make-circle 1 (make-pt 0 0)))
;;(setf c1 (make-circle 1 (make-pt 1 0)))
;;(setf c2 (make-circle 10 (make-pt 0 -10)))
;;(intersect c0 c1)
;;(intersect c0 c2)
;;(intersect c1 c2)

; PRINTING FUNCTIONS (stolen from the dating assignment but its useful here too)

; Helper function for "print-nicely" that prints the elements of the
; given list on separate lines.

(defun print-elements (list)
  (if (null list)
    nil
    (progn
      (print (car list))
      (print-elements (cdr list)))))


; Prints the list "list" with each element on a separate line.
; This function isn't really "functional" since its interesting
; output is essentially a side-effect to the console. It returns NIL
; because it needs to return something.  The read-eval-print loop
; in the Lisp interpreter will print out the NIL return value
; following the nice print out of the list.

(defun print-nicely (list)
  (if (not (listp list))
    (error "Function print-nicely received something other than a list")
    (progn
      (princ "(")
      (print-elements list)
      (terpri)
      (princ ")")
      (terpri)
      (terpri)
      nil)))

;;;;;;;;;;;;;;;;;End of Point/Circle functions;;;;;;;;;;;;;;;;;;;;;;;;;;

                  
; Given a list of distances and a list of stars, return a list of all
;;the possible guesses.  A single guess is a list of circles which pairs
;;each distance with one of the stars.
;;There must be as many stars as distances
;;This works a lot like permute, which I'll try to do
;;in lecture
(defun all-guesses (distances stars)
  (if (or (null distances) (null stars)) '(())
      (mapcan #'(lambda (star)
           (mapcar #'(lambda (pair) (cons (list (car distances) star) pair))
                  (all-guesses (cdr distances) (remove star stars))
                          )
                  )
            stars)))

;;its a good idea set up your own symbols with common test cases
;;as below--   Not having to retype the input
;;every time is a big improvement.

;;here is the final test
(defparameter *distances* '(2.65 5.55 5.25))
(defparameter *stars* '((0 0) (4 6) (10 0) (7 4) (12 5)))

;;; the "main" expression:
;;; (print-nicely (where-am-i *distances* *stars*))

;;or shorter names to save even more typing
(defparameter d1 *distances*)
(defparameter s1 *stars*)

;;here's a simpler test
(defparameter d2 '(2.5 11.65 7.75))
(defparameter s2 '((0 0) (4 4) (10 0)))

;;Include your code below.. make sure
;;write and test each function independently.  This
;;will require that you repeatedly type in
;;(load "whereami.lisp") at the command-prompt, allowing
;;it to override exisiting definitions and including
;;the most recently implemented into the lot.