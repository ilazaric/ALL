
(menu-bar-mode -1)
(scroll-bar-mode -1)
(tool-bar-mode -1)
(setq inhibit-startup-screen t)
(add-to-list 'default-frame-alist '(fullscreen . maximized))

(load-theme 'wombat)
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(default-text-scale-amount 16)
 '(default-text-scale-mode t nil (default-text-scale))
 '(markdown-command "pandoc --from markdown --to html5 --mathjax --standalone")
 '(org-support-shift-select t)
 '(package-selected-packages
   '(ejson-mode magit default-text-scale org-modern exec-path-from-shell dockerfile-mode pdf-tools latex-preview-pane auctex-latexmk auctex impatient-mode rust-mode markdown-mode cmake-font-lock cmake-mode go-mode)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

(require 'package)
(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/") t)
;; Comment/uncomment this line to enable MELPA Stable if desired.  See `package-archive-priorities`
;; and `package-pinned-packages`. Most users will not need or want to do this.
(add-to-list 'package-archives '("melpa-stable" . "https://stable.melpa.org/packages/") t)
;; (add-to-list 'package-archives
;;              '("melpa-stable" . "https://stable.melpa.org/packages/"))
(package-initialize)
;; (package-refresh-contents)

;; (require 'rust-mode)

(setq-default indent-tabs-mode nil)
(define-key global-map (kbd "C-x O") 'previous-window-any-frame)

(add-hook 'c++-mode-hook 'hs-minor-mode)
;; (add-hook 'c++-mode-hook 'lsp)
;; (add-hook 'c-mode-hook 'lsp)


;; zoom at startup, was 113 by default
(set-face-attribute 'default nil :height 240)
;; zoom in/out is global, via CM=, CM-, CM0 (last resets)
(require 'default-text-scale)
(default-text-scale-mode 1)


;; thanks chatgpt
(defun my/org-latex-scale-and-render ()
  "Save the buffer, set Org LaTeX previews to match current text scale, then re-render."
  (interactive)
  ;; 1) Save the current buffer
  (save-buffer)
  ;; 2) Derive a scale factor from the current `default` face height
  (let ((new-scale (/ (float (face-attribute 'default :height nil)) 100.0)))
    (setq org-format-latex-options
          (plist-put org-format-latex-options :scale new-scale)))
  ;; ;; 3) Re-render all LaTeX in the buffer; `t` forces image regeneration
  ;; (org-latex-preview (point-min) (point-max) t))
  (org-latex-preview '(64))
  (org-latex-preview '(16)))
(with-eval-after-load 'org
  (define-key org-mode-map (kbd "M-1") #'my/org-latex-scale-and-render))
;; (with-eval-after-load 'org
;;   (define-key org-mode-map (kbd "M-2") #'org-clear-latex-preview))

(with-eval-after-load 'org
  (defun my/org-clear-latex-preview (&optional beg end)
    "Interactive wrapper around `org-clear-latex-preview'.
Removes LaTeX preview overlays in region or entire buffer."
    (interactive "r")
    ;; If `org-clear-latex-preview' is missing or non-interactive,
    ;; you may see an error here.  Upgrade Org or define a fallback.
    (org-clear-latex-preview beg end))

  ;; Now bind our interactive wrapper to C-c C-x C-r in Org buffers
  (define-key org-mode-map (kbd "M-2") #'my/org-clear-latex-preview))
