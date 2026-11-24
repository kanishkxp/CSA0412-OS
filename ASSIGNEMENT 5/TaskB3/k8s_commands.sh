#!/usr/bin/env bash
# Commands for Task B3.3 (kind or minikube).
kind create cluster --name portal                    # or: minikube start
kind load docker-image simats/result-portal:1.0 --name portal   # minikube: minikube image load ...
kubectl apply -f deployment.yaml
kubectl get pods                                     # 3 pods Running
kubectl describe pod <name> | grep -A6 -E "Limits|Requests"
kubectl delete pod <name>                            # a new pod appears
kubectl get pods -w
kubectl apply -f hpa.yaml                            # needs metrics-server
kubectl get hpa -w &
hey -z 120s -c 100 http://localhost:8080/result/192511278   # after: kubectl port-forward svc/result-portal 8080:80
kubectl get pods                                     # up to 6 pods
